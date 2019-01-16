#include "tree.h"
#include <zjunix/slab.h>
#include <driver/vga.h>
#include <zjunix/utils.h>

#define NULL 0x0

/* initial the file tree
 * 
 *Function initial the file tree
 *@No return value
 */
void init_filetree()
{
    int i;
    root = (struct filetree *)kmalloc(sizeof(struct filetree)); /* apply for the memory space */
    root->child = NULL; /* the child node of root is NULL */
    root->parent = NULL; /* the parent node of root is NULL */
    for(i = 0; i < 256; i++)
    {
        root->name[i] = 0; /* clean the root's name */
    }
    root->name[0] = '/'; /* root's name is root directory */
    root->next = NULL;   /* the next node of root is NULL */
    root->before = NULL; /* the before node of root is NULL */
}

/* initial the node of tree
 * 
 *Function initial the node of tree, initial the child, parent, next, before and name
 *@param name filename
 *@return struct filetree*  the initial node of tree
 */
struct filetree * init_treenode(char * name)
{
    int i;
    struct filetree * p = (struct filetree *)kmalloc(sizeof(struct filetree)); /* apply for the memory space */
    p->child = NULL; /* the child node of new node is NULL */
    p->parent = NULL; /* the parent node of new node is NULL */
    p->next = NULL; /* the next node of new node is NULL */
    p->before = NULL; /* the before node of new node is NULL */
    for(i = 0; i < 256; i++)
    {
        p->name[i] = 0;
    }
    i = 0;
    do {
        p->name[i] = name[i]; /* assign the name of new node */
        i++;
    } while(name[i] != '\0' && i < 256);
    return p;
}

/* output the filename in the directory
 * 
 *Function output the all the name of file in the directory
 *@param name filename
 *@No return value
 */
void outputDir(struct filetree * tParent)
{
    struct filetree * p = tParent->child;
    /* if the node doesn't have child means that there is no file under the directory */
    while(tParent->child != NULL)
    {
        kernel_printf("%s",p->name);
        p = p->next;
    }
}

/* let one node becomes the child of other nodes
 * 
 *Function let one node becomes the child of other nodes
 *@param tParent the parent node
 *@param tChild the child node
 *@No return value
 */
void becomeChild(struct filetree * tParent, struct filetree * tChild)
{
    /* if the parent doesn't have child, then the new node become the child of parent */
    if (tParent->child == NULL)
    {
        tParent->child = tChild;
        tChild->parent = tParent;
    }
    /* if the parent has child, then the new node become the brother of the child of parent */
    else 
    {
        tChild->next = tParent->child;
        tParent->child->before = tChild;
        tParent->child = tChild;
        tChild->parent = tParent;
    }
}

/* find node whose name is param on the tree
 * 
 *Function find the node whose name is param
 *@param param is the name of the node
 *@return struct filetree * the node which we find
 */
struct filetree * findNode (char * param)
{
    struct filetree * p;
    struct filetree * c;
    p = root;
    while(p != NULL)
    {
        c = p;
        if(matching(c,param))
        {
            return c;
        }
        else
        {
            while(c->next != NULL)
            {
                c = c->next;
                if(matching(c,param))
                {
                    return c;
                }
            }
            p = p->child;
        }
    }
    return NULL;
}

/* delele one node on the tree
 * 
 *Function delete one node on the tree whose name is param
 *@param param the name of file
 *@No return value
 */
void deleteNode(char * param)
{
    struct filetree * p;
    struct filetree * t1;
    struct filetree * t2;
    p = findNode(param);
    if(p == NULL)
    {
    }
    else
    {
        if(p->parent == NULL)
        {
            kernel_printf("parent is NULL\n");
            kfree(p);
        }
        else
        {
            t1 = p->next;
            t2 = p->before;
            if(t2 == NULL)
            {
                p->parent->child = t1;
                if(t1 != NULL)
                {
                    t1->before = NULL;
                }
            }
            else
            {
                t2->next = t1;
                if(t1 != NULL)
                {
                    t1->before = t2;
                }
            }
        }
    }
}

/* print all the node on the tree
 * 
 *Function print the all the node on the tree, the value we print is the name of file
 *@param ft is the root of tree
 *@No return value
 */
void print_tree(struct filetree * ft)
{
    struct filetree * p;
    struct filetree * c;
    p = ft;
    /* from the root and to the next, then output the name of node in the same level */
    while(p != NULL)
    {
        c = p;
        kernel_printf("    %s\n", c->name);
        /* for in the same level */
        while(c->next != NULL)
        {
            c = c->next;
            kernel_printf("    %s\n", c->name);
        }
        p=p->child;
    }
}

/* find the tree is empty or not
 * 
 *Function try to find whether the tree is empty or not
 *@param ft is the root of tree
 *@return int 1 means the tree is empty, 0 means the tree is not empty
 */
int Empty(struct filetree * ft)
{
    /* if the tree is empty */
    if(ft->child == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* whether the name is match with the node's name
 * 
 *Function find whether there is a node which matches the filename on the tree
 *@param input the filename
 *@return int 1 means it is matched, 0 means it is not matched.
 */
int matching(struct filetree * p, char * param)
{
    int i;
    unsigned int len1 = 0;
    unsigned int len2 = 0;
    /* calculate the length of the name */
    while (p->name[len1])
        ++len1;
    /* calculate the length of the name */
    while (param[len2])
        ++len2;
    /* if the length of name is not equal, then their name is not the same */
    if(len1 != len2)
    {
        return 0;
    }
    else
    {
        for(i=0; i<len1; i++)
        {
            if(p->name[i] == param[i])
            {
            }
            else
            {
                break;
            }
        }
        if(i < len1 - 1)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
}
