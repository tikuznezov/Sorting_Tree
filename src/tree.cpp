#include     <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include      <time.h>
#include      <math.h>
#include   <stdbool.h>
#include    <assert.h>
#include  <sys/stat.h>
#include    <unistd.h>
#include     <fcntl.h>
#include <sys/types.h>
#include     <ctype.h>
#include    <stdarg.h>

// #define _DEBUG
// #define _FUNCTION_STATE
#include "log.cpp"
#include "term_colors.h"

typedef double tree_element_t;
#define PR_TREE_EL "%-8lg"
const tree_element_t POISON = 67676767;


// Defines and structures
//-------------------------------------------------------------------------------------------

enum ERR_CODE
{
    UNKNOWN_ERROR         = -1,
    NO_ERROR              = 0,
    GET_NULL_TREE_POINTER = 't'+'r'+'e'+'e',
    GET_NULL_NODE_POINTER,
    PRINT_NODE_RECURSION_LIMIT
};

struct TreeN
{
    tree_element_t data;
    TreeN * parent;
    TreeN * left;
    TreeN * right;
};

enum TREE_SIDES
{
    RIGHT = 0,
    LEFT = 1
};

struct Tree
{
    TreeN * root;
    size_t element_count;
};    

// GraphViz
//-------------------------------------------------------------------------------------------

FILE *gv = NULL;

#define fp(string, ...)  fprintf(gv, string, ## __VA_ARGS__);

const size_t SYS_COMMAND_LENGTH = 100;

void CompPng(const char *name)
{
    static size_t comp_pic_number = 1;
    extern FILE *gv;
    fclose(gv);
        gv = NULL;
        char sys_com[SYS_COMMAND_LENGTH];
        snprintf(sys_com, SYS_COMMAND_LENGTH, "dot -Tsvg Tree.gv -o pictures/%s_%zu.svg", name, comp_pic_number++);
        system(sys_com);
}

// recursion
const size_t NODE_RECURSION_LIMIT = 10000;
int PrintTreeNode(TreeN *node)
{
    static size_t recursion_iter = 0;

    if (node != NULL)
    {
        // Create node
        fp("\n\tn%p [label=\"{Tree element %p.| arg = " PR_TREE_EL " | parent = %p |{ left = %p | right = %p }}\"];\n", node, node, node->data, node->parent, node->left, node->right);
        fp(  "\tn%p:n -> n%p:s [color = green];\n", node, node->parent);

        // Left descendant
        if (node->left == NULL)
        {
            // Create NULL descendant
            fp("\tNULL_L_%p [label=\"NULL\", fillcolor = \"#000000\"];\n", node);
            fp("\tn%p -> NULL_L_%p;\n", node, node);
        }
        else
        {
            fp("\tn%p -> n%p;\n", node, node->left);
        }

        // Right descendant
        if (node->right == NULL)
        {
            // create NULL descendant
            fp("\tNULL_R_%p [label=\"NULL\", fillcolor = \"#000000\"];\n", node);
            fp("\tn%p -> NULL_R_%p;\n", node, node);
        }
        else
        {
            fp("\tn%p -> n%p;\n", node, node->right);
        }

        // break if infinite recursion
        if (++recursion_iter > NODE_RECURSION_LIMIT)
        {
            printf(PRED "--> Recursion call limit has been reached in %s after %zu iterations!\n" "Limit = %zu.\n" DEF_COL, __FUNCTION__, recursion_iter, NODE_RECURSION_LIMIT);
            global_error = PRINT_NODE_RECURSION_LIMIT;
            return PRINT_NODE_RECURSION_LIMIT;
        }
        // continue recursion
        else
        {
            PrintTreeNode(node->left );
            PrintTreeNode(node->right);
        }
    }

    return 0;
}

// run recursion
#define PrintTreeGV(tree_p) PrintTreeGV_DEF(tree_p, __FUNCTION__)
int PrintTreeGV_DEF(const Tree * const tree, const char *name)
{
    PR_FST;
    extern FILE *gv;
    gv = fopen("Tree.gv", "w");
    
    PR("Start printing to GV...");

    // graph info
    fp("\n\ndigraph %s {\n", name);
    fp("rankdir=TB; splines=ortho; nodesep = 1; bgcolor=\"#FFFFFF\";\n");
    fp("node [shape = Mrecord, style = filled, fillcolor = \"#626ae0\", fontcolor = \"#ffffff\", fontname=\"Helvetica-Bold\"];\n");

    // zero node
    fp("\n# ZERO NODE\n\tn0x0 [label=\"Root parent\"];\n");

    // tree nodes
    PrintTreeNode(tree->root);

    fp("}");

    CompPng("tree_pic");

    PR(" Completed.\n\n");

    PR_FED;
    return 0;
}

// Functions
//-------------------------------------------------------------------------------------------

void TreeVerify(const Tree * const tree)
{
    PR_FST;
    NAMED_ASSERT(tree, GET_NULL_TREE_POINTER, "tree");

    #ifdef _DEBUG
        if (tree->root == NULL)
        {
            PR("* Tree root is NULL pointer. Tree is void.\n");
            return;
        }

        PrintTreeGV(tree);
    #endif

    PR_FED;
    return;
}


void PrintNodeAsTable(const TreeN * const node_p)
{
    NAMED_ASSERT(node_p, GET_NULL_NODE_POINTER, "node");

    PR("|-----------------------|\n");
    PR("| POINTER: "     "%-12p" " |\n",    node_p);
    PR("|-----------------------|\n");
    PR("|    DATA: " PR_TREE_EL "     |\n", node_p->data);
    PR("|-----------------------|\n");
    PR("|    LEFT: "     "%-12p" " |\n",    node_p->left);
    PR("|-----------------------|\n");
    PR("|   RIGHT: "     "%-12p" " |\n",    node_p->right);
    PR("|-----------------------|\n");
    PR("\n");


    return;
}

void PrintNode(const TreeN * const node_p)
{
    PR_FST;
    NAMED_ASSERT(node_p, GET_NULL_NODE_POINTER, "node");

    PR("Tree node [Address = %p; Data = " PR_TREE_EL "; Left = %p; Right = %p]\n", node_p, node_p->data, node_p->left, node_p->right);
    PrintNodeAsTable(node_p);

    PR_FED;
    return;
}

int TreeCreator (Tree *tree)
{
    PR_FST;
    tree->root = NULL;
    tree->element_count = 0;

    TreeVerify(tree);

    PR_FED;
    return global_error;
}

TreeN * NewNode (tree_element_t arg)
{
    PR_FST;
    TreeN *new_node_address = (TreeN *) calloc(1, sizeof(TreeN));
    if (new_node_address == NULL)
    {
        perror("Can't get memory to new node.");
        return new_node_address;
    }
    assert(new_node_address);

    *new_node_address = {.data = arg, .parent = NULL, .left = NULL, .right = NULL};
    // PrintNode(new_node_address);

    PR_FED;
    return new_node_address;
}

TreeN * FindTreeNodeByValueREC(TreeN *root_node, tree_element_t arg)
{
    // NAMED_ASSERT(root_node, GET_NULL_TREE_POINTER, "tree");
    TreeN *result = NULL;

    if (root_node != NULL)
    {
        if (root_node->data == arg) 
            result = root_node;
        if (result == NULL)
            result = FindTreeNodeByValueREC(root_node->left, arg);
        if (result == NULL)
            result = FindTreeNodeByValueREC(root_node->right, arg);
    }

    return result;
}

TreeN * FinderInTree(Tree *tree, tree_element_t find_val)
{
    PR_FST;
    TreeVerify(tree);

    TreeN *find_pointer = NULL;
    PR("Finding <" PR_TREE_EL "> element...\n", find_val);

    find_pointer = FindTreeNodeByValueREC(tree->root, find_val);

    if (find_pointer == NULL)
    {
        printf("Failed to find <" PR_TREE_EL "> in tree. Element doesn't exist.\n", find_val);
    }
    else
    {
        printf("Element <" PR_TREE_EL "> successfully found.\n", find_pointer->data);
    }

    PR_FED;
    return find_pointer;
}

int TreeAdd     (Tree *tree, tree_element_t arg, TreeN *parent, const TREE_SIDES side)
{
    PR_FST;
    TreeVerify(tree);

    PR("Adding <" PR_TREE_EL "> to the %s side...\n", arg, ((side == LEFT) ? "left" : "right"));
    PR("Parent name = \"%p\"\n", parent);
    // tree is void
    if (tree->root == NULL)
    {
        PR("Tree haven'n root. Creating root node.\n");
        tree->root = NewNode(arg);
    }
    else if (parent != NULL)
    {
        NAMED_ASSERT(parent, GET_NULL_NODE_POINTER, "PARENT NODE");
        if (((side == LEFT) ? parent->left : parent->right) == NULL)
        //  (select side to link new node                   link new node) -> add parent link;
            ((side == LEFT) ? parent->left : parent->right = NewNode(arg)) -> parent = parent;
        else
            printf("Node already exists. Adding new node has been broke.\n");
    }
    else
    {
        PR("Parent is null pointer. Crush addiction (" PR_TREE_EL ") node to tree.\n", arg);
    }

    TreeVerify(tree);
    PR_FED;
    return global_error;
}


#define LINK_FROM_PARENT_TO_DEC ((del_node != tree->root)                                                                         ? \
                                 (((del_node->parent)->left == del_node) ? (del_node->parent)->left : (del_node->parent)->right)  : \
                                 (tree->root)                                                                                        )

int TreeDelete  (Tree *tree, TreeN *del_node, const TREE_SIDES replace)
{
    PR_FST;
    TreeVerify(tree);
    if (del_node == NULL)
    {
        printf("Try to delete node by <%p> address (or node doesn't exist).\n", del_node);
        return 0;
    }
    NAMED_ASSERT(del_node, GET_NULL_NODE_POINTER, "DELETED NODE");

    PR("Deleting node %p. \nParent = %p; Left = %p; Right = %p;\n", del_node, del_node->parent, del_node->left, del_node->right);



    if ((del_node->left == NULL) && (del_node->right == NULL))
    {
        PR("Deleting node without descendants...");
        // delete parent link
        LINK_FROM_PARENT_TO_DEC = NULL;
    }
    else if (del_node->left == NULL)
    {
        PR("Deleting node with only right descendant...");
        LINK_FROM_PARENT_TO_DEC = del_node->right;
        del_node->right->parent = del_node->parent;
    }
    else if (del_node->right == NULL)
    {
        PR("Deleting node with only left descendant...");
        LINK_FROM_PARENT_TO_DEC = del_node->left;
        del_node->left ->parent = del_node->parent;
    }
    else if ((del_node->left != NULL) && (del_node->right != NULL))
    {
        PR("Deleting node with two descendants...");
        if (replace == LEFT)
        {
            LINK_FROM_PARENT_TO_DEC = del_node->left;
            del_node->left->right   = del_node->right;
            del_node->right->parent = del_node->left;
            del_node->left->parent  = del_node->parent;
        }
        else if (replace == RIGHT)
        {
            LINK_FROM_PARENT_TO_DEC  = del_node->right;
            del_node->right->left    = del_node->left;
            del_node->left->parent   = del_node->right;
            del_node->right->parent  = del_node->parent;
        }
    }
    else
    {
        PR("--> Unknown delete case! Break deleting.\n");
        return -1;
    }

    *del_node = {};
    free(del_node);

    PR("\tDeleting finished.\n");
    TreeVerify(tree);
    return global_error;
}



// Main
//-------------------------------------------------------------------------------------------

int main(void)
{
    printf(PGREEN "------------------ RUN MAIN ------------------\n" DEF_COL);

    Tree tree1 = {};
    TreeCreator(&tree1);

    TreeAdd(&tree1, 52, NULL, RIGHT);

    TreeN *p67 = FinderInTree(&tree1, 67);

    TreeAdd(&tree1, 67, FinderInTree(&tree1, 52), RIGHT);
    TreeAdd(&tree1, 1488.12312, FinderInTree(&tree1, 67), RIGHT);
    TreeAdd(&tree1, 6752, FinderInTree(&tree1, 67), RIGHT);
    TreeAdd(&tree1, 111, FinderInTree(&tree1, 67), LEFT);

    PrintTreeGV(&tree1);

    TreeDelete(&tree1, FinderInTree(&tree1, 67), RIGHT);
    TreeDelete(&tree1, FinderInTree(&tree1, 52), RIGHT);
    TreeDelete(&tree1, FinderInTree(&tree1, 52), RIGHT);

    PrintTreeGV(&tree1);


    printf(PYELLOW "------------------ END MAIN ------------------\n" DEF_COL);
    return 0;
}