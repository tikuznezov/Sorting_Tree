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

#define _DEBUG
// #define _FUNCTION_STATE
#include "log.cpp"
#include "term_colors.h"

typedef int tree_element_t;
#define TREE_SPEC "%d"


// Defines and structures
//-------------------------------------------------------------------------------------------

enum ERR_CODE
{
    UNKNOWN_ERROR         = -1,
    NO_ERROR              = 0,
    GET_NULL_TREE_POINTER = 't'+'r'+'e'+'e',
    GET_NULL_NODE_POINTER,
    PRINT_NODE_RECURSION_LIMIT,
    FILE_DOES_NOT_EXIST,
    GET_NULL_STRING_TO_READ,
    INCORRECT_READ_FILE_FORMAT
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
        fp("\n\tn%p [label=\"{Tree element %p.| arg = " TREE_SPEC " | parent = %p |{ left = %p | right = %p }}\"];\n", node, node, node->data, node->parent, node->left, node->right);
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
    PR("|    DATA: " TREE_SPEC "     |\n", node_p->data);
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

    PR("Tree node [Address = %p; Data = " TREE_SPEC "; Left = %p; Right = %p]\n", node_p, node_p->data, node_p->left, node_p->right);
    PrintNodeAsTable(node_p);

    PR_FED;
    return;
}

Tree * TreeCreator ()
{
    PR_FST;
    Tree *tree = (Tree *) calloc(1, sizeof(Tree));

    tree->root = NULL;
    tree->element_count = 0;

    TreeVerify(tree);

    PR_FED;
    return tree;
}

void TreeDestructor(Tree *tree)
{
    NAMED_ASSERT(tree, GET_NULL_TREE_POINTER, "tree to delete");

    *tree = {};
    free(tree);

    return;
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
    PR("Finding <" TREE_SPEC "> element...\n", find_val);

    find_pointer = FindTreeNodeByValueREC(tree->root, find_val);

    if (find_pointer == NULL)
    {
        printf("Failed to find <" TREE_SPEC "> in tree. Element doesn't exist.\n", find_val);
    }
    else
    {
        printf("Element <" TREE_SPEC "> successfully found.\n", find_pointer->data);
    }

    PR_FED;
    return find_pointer;
}

int TreeAdd     (Tree *tree, tree_element_t arg, TreeN *parent, const TREE_SIDES side)
{
    PR_FST;
    TreeVerify(tree);

    PR("Adding <" TREE_SPEC "> to the %s side...\n", arg, ((side == LEFT) ? "left" : "right"));
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
            (((side == LEFT) ? parent->left : parent->right) = NewNode(arg)) -> parent = parent;
        else
            printf("Node already exists. Adding new node has been broke.\n");
    }
    else
    {
        PR("Parent is null pointer. Crush addiction (" TREE_SPEC ") node to tree.\n", arg);
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

int WriteNode   (FILE *file, const TreeN *node)
{
    NAMED_ASSERT(file, FILE_DOES_NOT_EXIST, "FILE NAME");

    fprintf(    file, "("); // FIXME
    if (node == NULL)
    {
        fprintf(file, "NIL");
    }
    else
    {
        NAMED_ASSERT(node, GET_NULL_NODE_POINTER, "printing node");

        fprintf(    file, TREE_SPEC, node->data);
        // fprintf(    file, "(");
        WriteNode(  file, node-> left);
        // fprintf(    file, ")(");
        WriteNode(  file, node->right);
        // fprintf(    file, ")");
    }
    fprintf(    file, ")"); // FIXME

    return 0;
}

int WriteTree   (const char *file_name, const Tree  *tree)
{
    NAMED_ASSERT(file_name, FILE_DOES_NOT_EXIST, "FILE NAME");
    TreeVerify(tree);

    FILE *writing_file = fopen(file_name, "w");

    WriteNode(writing_file, tree->root);

    fclose(writing_file);

    return 0;
}

#define ARGUMENT_SPEC "%[A-Za-z _-,.?!;:' 0-9 \"\t]"
TreeN *ReadNode    (char *tree_data, const Tree *tree, size_t *shift, TreeN *parent)
{
    NAMED_ASSERT(tree_data, GET_NULL_STRING_TO_READ, "read string");
    PR("Read node (tree data pointer = %p, tree pointer = %p, shift = %zu)\n", tree_data, tree, *shift);
    char parentheses[2]   = "";
    char arg[MAX_STR_LEN] = "";
    int temp_shift;
    TreeN *new_node = NULL;

    // как такие инициализировать?
    tree_element_t node_data;

    PR("Check opend \"(\".");
    if (1 == sscanf(tree_data + *shift, "%1[(]%n", parentheses, &temp_shift))
    {
        *shift += (size_t)temp_shift;
        PR(" Skip %d symbols.\n", temp_shift);

        // Read node data
        if (1 != sscanf(tree_data + *shift, ARGUMENT_SPEC"%n", arg, &temp_shift))
        {
            PR(PRED "Error in reading node argument. Incorrect format.\n" DEF_COL);
            global_error = INCORRECT_READ_FILE_FORMAT;
        }
        *shift += (size_t)temp_shift;
        PR("Get \"%s\". Skip %d symbols.\n", arg, temp_shift);

        if (!strncmp(arg, "NIL", 4))
        {
            PR("Get NIL. Return NULL;\n");
            new_node = NULL;
        }
        else if (1 == sscanf(arg, TREE_SPEC, &node_data))
        {
            PR("Creation new node...");
            new_node = NewNode(node_data);
            new_node->parent = parent;
            PR(" Completed.\n");
            PR("Call left node...");
            new_node->left  = ReadNode(tree_data, tree, shift, new_node);
            PR("Completed.\n");
            PR("Call right node...");
            new_node->right = ReadNode(tree_data, tree, shift, new_node);
            PR("Completed.\n");
        }
    }
    if (1 != sscanf(tree_data + *shift, "%1[)]%n", parentheses, &temp_shift))
    {
        PR(PRED "Read file format is incorrect!\n" DEF_COL);
        global_error = INCORRECT_READ_FILE_FORMAT;
    }
    *shift += (size_t)temp_shift;

    #ifdef _DEBUG
        Tree local_tree = {};
        local_tree.root = new_node;
        PrintTreeGV(&local_tree);
    #endif

    return new_node;
}

Tree *ReadTree    (const char *file_name)
{
    NAMED_ASSERT(file_name, FILE_DOES_NOT_EXIST, "FILE NAME");

    Tree *tree = TreeCreator();
    TreeVerify(tree);

    size_t file_size = (size_t) SizeofFile(file_name);
    PR("File size = %zu;\n", file_size);

    int tree_file    = open(file_name, O_RDONLY);
    if (tree_file == -1)
        perror("Tree file wasn't opened.");
    PR("file opend.\n");

    char *tree_data  = (char *) calloc(file_size + 2, sizeof(char));
    if (tree_data == NULL)
        perror("Can't get enough memory to read file.");
    PR("Buffer created.\n");

    if (read(tree_file, tree_data, file_size) == -1)
        perror("Unsuccessful read file.");
    PR("File read.\n");

    size_t shift = 0;
    tree->root = ReadNode(tree_data, tree, &shift, NULL);

    return tree;
}

TreeN *FindNearParent(TreeN *node, tree_element_t arg)
{
    if (node == NULL)
        return NULL;

    if      (node->data < arg)
    {
        if (node->right == NULL) return node;
        return FindNearParent(node->right, arg);
    }
    else if (node->data > arg)
    {
        if (node->left == NULL) return node;
        return FindNearParent(node->left, arg);
    }
    else if (node->data == arg)
    {
        printf("Element in tree.\n");
        return NULL;
    }

    return NULL;
}

int TreeSortAdd   (Tree *tree, tree_element_t arg)
{
    PR_FST;
    TreeVerify(tree);

    if (tree->root == NULL)
    {
        TreeAdd(tree, arg, NULL, RIGHT);
        return 0;
    }

    TreeN *parent_node = FindNearParent(tree->root, arg);
    if (parent_node == NULL)
    {
        PR("Node not found or already exist.\n");
        return 0;
    }

    if (arg > parent_node->data)
    {
        TreeAdd(tree, arg, parent_node, RIGHT);
    }
    else
    {
        TreeAdd(tree, arg, parent_node, LEFT);
    }

    PR_FED;
    return 0;
}

// Main
//-------------------------------------------------------------------------------------------

int main(void)
{
    printf(PGREEN "------------------ RUN MAIN ------------------\n" DEF_COL);

    Tree *tree1 = TreeCreator();

    // TreeAdd(&tree1, 52, NULL, RIGHT);

    // TreeAdd(&tree1, 67, FinderInTree(&tree1, 52), RIGHT);
    // TreeAdd(&tree1, 1488, FinderInTree(&tree1, 67), RIGHT);
    // TreeAdd(&tree1, 6752, FinderInTree(&tree1, 67), RIGHT);
    // TreeAdd(&tree1, 111, FinderInTree(&tree1, 67), LEFT);

    // WriteTree("porno.txt", &tree1);
    // PrintTreeGV(&tree1);

    // TreeDelete(&tree1, FinderInTree(&tree1, 67), RIGHT);
    // TreeDelete(&tree1, FinderInTree(&tree1, 52), RIGHT);
    // TreeDelete(&tree1, FinderInTree(&tree1, 52), RIGHT);

    // WriteTree("porno1.txt", &tree1);
    // PrintTreeGV(&tree1);

    // TreeDelete(&tree1, tree1.root, RIGHT);
    // WriteTree("porno2.txt", &tree1);
    // PrintTreeGV(&tree1);

    // TreeDelete(&tree1, tree1.root, RIGHT);
    // WriteTree("porno3.txt", &tree1);
    // PrintTreeGV(&tree1);

    // TreeDelete(&tree1, tree1.root, RIGHT);
    // WriteTree("porno4.txt", &tree1);
    // PrintTreeGV(&tree1);


    // ReadTree("porno.txt", &tree1);
    printf(PGREEN "%p;\n" DEF_COL, FindNearParent(tree1->root, 32));
    TreeSortAdd(tree1, 67);
    for (int i = 0; i < 50; i++)
    {
        TreeSortAdd(tree1, rand()%1000);
    }
    // PrintTreeGV(tree1);

    WriteTree("porno.txt", tree1);
    getchar();
    Tree *porno = ReadTree("porno.txt");
    PrintTreeGV(porno);

    printf(PYELLOW "------------------ END MAIN ------------------\n" DEF_COL);
    return 0;
}