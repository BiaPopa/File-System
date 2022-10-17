#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."

char *mystrdup(const char *str)
{
    // aloca memorie si face o copie stringului str
    int n = strlen(str) + 1;
    char *dup = malloc(n);
    if (dup != NULL)
        strcpy(dup, str);
    return dup;
}

// functie care creeaza contentul unui TreeNode de tip file
FileContent *create_FileContent(char *info)
{
    FileContent *cnt = malloc(sizeof(struct FileContent));

    cnt->text = info;

    return cnt;
}

// functie care creeaza contentul unui TreeNode de tip folder
FolderContent *create_FolderContent()
{
    FolderContent *cnt = malloc(sizeof(struct FolderContent));

    cnt->children = malloc(sizeof(struct List));
    cnt->children->head = NULL;

    return cnt;
}

// functie care creeaza un TreeNode
TreeNode *create_TreeNode(TreeNode *parent, char *name,
                        unsigned int type, char *info)
{
    TreeNode *node =  malloc(sizeof(struct TreeNode));

    node->parent = parent;

    node->name = name;
    if (type == 0) {
        node->type = FILE_NODE;
        node->content = create_FileContent(info);
    } else {
        node->type = FOLDER_NODE;
        node->content = create_FolderContent();
    }

    return node;
}

FileTree createFileTree(char* rootFolderName) {
    FileTree tree;
    // se creeaza folderul root
    tree.root = malloc(sizeof(struct TreeNode));

    tree.root->parent = NULL;
    tree.root->name = rootFolderName;
    tree.root->type = FOLDER_NODE;
    tree.root->content = create_FolderContent();

    return tree;
}

// functie care elibereaza contentul unui TreeNode de tip file
void free_file(FileContent *txt)
{
    free(txt->text);
    free(txt);
}

// functie care elibereaza contentul unui TreeNode de tip folder
void free_folder(FolderContent *ll)
{
    ListNode *node = ll->children->head;
    ListNode *next;
    while (node != NULL) {
        next = node->next;
        free_TreeNode(node->info);
        free(node);
        node = next;
    }
    free(ll->children);
    free(ll);
}

// functie care elibereaza un TreeNode
void free_TreeNode(TreeNode *tree)
{
    if (tree->type == 0) {
        FileContent *tmp = tree->content;
        free_file(tmp);
    } else {
        FolderContent *tmp = tree->content;
        free_folder(tmp);
    }
    free(tree->name);
    free(tree);
}

void freeTree(FileTree fileTree) {
    TreeNode *tree = fileTree.root;
    free_TreeNode(tree);
}

// functie care afiseaza in ordine inversa elementele din lista
void print_dir(ListNode *node)
{
    if (node == NULL)
        return;

    print_dir(node->next);
    printf("%s\n", node->info->name);
}

void ls(TreeNode* currentNode, char* arg) {
    FolderContent *cnt = currentNode->content;
    ListNode *node = cnt->children->head;

    if (strcmp(arg, "\0") == 0) {
        // daca comanda nu are parametri, se afiseaza
        // resursele directorului curent
        print_dir(node);
    } else {
        // se verifica daca exista un fisier/director
        // cu acelasi nume
        while (node != NULL) {
            if (strcmp(node->info->name, arg) == 0) {
                if (node->info->type == 0) {
                    // daca e fisier, se afiseaza cotinutul text
                    FileContent *txt = node->info->content;
                    printf("%s: %s\n", node->info->name, txt->text);
                } else if (node->info->type == 1) {
                    // daca e director, se afiseaza resursele lui
                    FolderContent *ll = node->info->content;
                    ListNode *node2 = ll->children->head;
                    print_dir(node2);
                }
                return;
            }
            node = node->next;
        }
        // daca nu exista un fisier/director cu acelasi
        // nume, se afiseaza eroarea
        printf("ls: cannot access '%s': No such file or directory\n", arg);
    }
}


void pwd(TreeNode* treeNode) {
    // afiseaza recursiv calea absoluta catre treeNode
    if (treeNode->parent == NULL) {
        printf("root");
        return;
    }

    pwd(treeNode->parent);
    printf("/%s", treeNode->name);
}


TreeNode* cd(TreeNode* currentNode, char* path) {
    // se salveaza o copie a lui path
    char *copy = malloc(strlen(path) + 1);
    strcpy(copy, path);

    TreeNode *curr = currentNode;
    FolderContent *cnt = currentNode->content;
    ListNode *node = cnt->children->head;

    // se parcurge calea
    char *p = strtok(path, "/");
    while (p != NULL) {
        // daca p e .. se face referire la directorul parinte
        if (strcmp(p, "..") == 0) {
            if (currentNode->parent != NULL) {
                currentNode = currentNode->parent;
                cnt = currentNode->content;
                node = cnt->children->head;
            } else {
                // daca nu exista director parinte, atunci se
                // afiseaza eroarea
                printf("cd: no such file or directory: %s\n", copy);
                free(copy);
                return curr;
            }
        } else {
            int ok = 0;
            // se parcurg resursele directorului curent
            while (node != NULL) {
                if (strcmp(node->info->name, p) == 0 && node->info->type == 1) {
                    // daca se gaseste resursa cu numele p si este de tip
                    // folder, atunci se avanseaza in ierarhia de fisiere
                    currentNode = node->info;
                    cnt = currentNode->content;
                    node = cnt->children->head;
                    ok = 1;
                    break;
                }
                node = node->next;
            }

            // daca nu exista directorul p prin resursele directorului curent,
            // atunci se afiseaza eroarea
            if (ok == 0) {
                printf("cd: no such file or directory: %s\n", copy);
                free(copy);
                return curr;
            }
        }
        p = strtok(NULL, "/");
    }
    free(copy);
    // se returneaza directorul destinatie
    return currentNode;
}

// functie care afiseaza ierarhia de fisiere pornind de la head-ul din
// lista de resurse a directorului curent
void print_tree(ListNode *node, int *spaces, int *nrdir, int *nrfiles)
{
    if (node == NULL) {
        return;
    }
    // lista de resurse este afisata in ordine inversa
    print_tree(node->next, spaces, nrdir, nrfiles);

    int nr = *spaces;
    while (nr) {
       printf("\t");
        --nr;
    }
    // se afiseaza resursa
    printf("%s\n", node->info->name);

    if (node->info->type == 1) {
        // daca resursa afisata este director, atunci se
        // afiseaza si resursele acesteia
        ++(*nrdir);
        FolderContent *cnt = node->info->content;
        ListNode *node2 = cnt->children->head;
        ++(*spaces);
        print_tree(node2, spaces, nrdir, nrfiles);
        --(*spaces);
    } else {
        ++(*nrfiles);
    }
}

void tree(TreeNode* currentNode, char* arg) {
    int nrdir = 0, nrfiles = 0, spaces = 0;
    if (strcmp(arg, "\0") == 0) {
        // daca comanda nu are parametri, se afiseaza
        // ierahia de fisiere pornind din directorul curent
        FolderContent *cnt = currentNode->content;
        ListNode *node = cnt->children->head;
        print_tree(node, &spaces, &nrdir, &nrfiles);
        printf("%d directories, %d files\n", nrdir, nrfiles);

    } else {
        // se salveaza o copie a lui path
        char *copy = malloc(strlen(arg) + 1);
        strcpy(copy, arg);
        FolderContent *cnt = currentNode->content;
        ListNode *node = cnt->children->head;

        // se parcurge calea
        char *p = strtok(arg, "/");
        while (p != NULL) {
            // daca p e .. se face referire la directorul parinte
            if (strcmp(p, "..") == 0) {
                if (currentNode->parent != NULL) {
                    currentNode = currentNode->parent;
                    cnt = currentNode->content;
                    node = cnt->children->head;
                } else {
                    // daca nu exista director parinte, atunci se
                    // afiseaza eroarea
                    printf("%s [error opening dir]\n\n", copy);
                    printf("0 directories, 0 files\n");
                    free(copy);
                    return;
                }
            } else {
                int ok = 0;
                // se parcurg resursele directorului curent
                while (node != NULL) {
                    if (strcmp(node->info->name, p) == 0 &&
                        node->info->type == 1) {
                        // daca se gaseste resursa cu numele p si este de tip
                        // folder, atunci se avanseaza in ierarhia de fisiere
                        currentNode = node->info;
                        cnt = currentNode->content;
                        node = cnt->children->head;
                        ok = 1;
                        break;
                    }
                    node = node->next;
                }

                // daca nu exista directorul p prin resursele directorului
                // curent, atunci se afiseaza eroarea
                if (ok == 0) {
                    printf("%s [error opening dir]\n\n", copy);
                    printf("0 directories, 0 files\n");
                    free(copy);
                    return;
                }
            }
            p = strtok(NULL, "/");
        }
        free(copy);
        // se apeleaza functia de afisare
        cnt = currentNode->content;
        node = cnt->children->head;
        print_tree(node, &spaces, &nrdir, &nrfiles);
        printf("%d directories, %d files\n", nrdir, nrfiles);
    }
}


void mkdir(TreeNode* currentNode, char* folderName) {
    FolderContent *cnt = currentNode->content;

    if (cnt->children->head == NULL) {
        // daca lista de resurse a directorului curent este goala
        // atunci directorul care se creeaza este head-ul
        cnt->children->head = malloc(sizeof(struct ListNode));
        unsigned int type = 1;
        cnt->children->head->info =
            create_TreeNode(currentNode, folderName, type, NULL);
        cnt->children->head->next = NULL;
        return;
    }

    // directorul care se creeaza este adaugat la sfarsitul listei
    ListNode *node = cnt->children->head;

    while (node->next != NULL) {
        // se parcurge lista si se verifica daca directorul exista deja
        if (strcmp(node->info->name, folderName) == 0) {
            // daca exista, atunci se afiseaza eroarea
            printf("mkdir: cannot create directory '%s': ", folderName);
            printf("File exists\n");
            return;
        }
        node = node->next;
    }
    if (strcmp(node->info->name, folderName) == 0) {
        printf("mkdir: cannot create directory '%s': ", folderName);
        printf("File exists\n");
        return;
    }
    node->next = malloc(sizeof(struct ListNode));
    unsigned int type = 1;
    node->next->info = create_TreeNode(currentNode, folderName, type, NULL);
    node->next->next = NULL;
}


void rmrec(TreeNode* currentNode, char* resourceName) {
    FolderContent *cnt = currentNode->content;
    ListNode *node = cnt->children->head;
    ListNode *prev = NULL;

    // se parcurge lista pana se gaseste resursa resourceName
    while (node != NULL) {
        if (strcmp(node->info->name, resourceName) == 0) {
            if (prev == NULL) {
                // daca resursa este head-ul listei, atunci
                // head-ul devine urmatoarea resursa
                cnt->children->head = node->next;
                // se elibereaza memoria
                free_TreeNode(node->info);
                free(node);
            } else {
                // daca resursa nu este head, atunci resursa anterioara
                // va indica catre resursa posterioara resursei curente
                prev->next = node->next;
                // se elibereaza memoria
                free_TreeNode(node->info);
                free(node);
            }
            return;
        }
        prev = node;
        node = node->next;
    }
    // daca nu exista resursa resourceName, se afiseaza eroarea
    printf("rmrec: failed to remove '%s': ", resourceName);
    printf("No such file or directory\n");
}


void rm(TreeNode* currentNode, char* fileName) {
    FolderContent *cnt = currentNode->content;
    ListNode *node = cnt->children->head;
    ListNode *prev = NULL;

    // se parcurge lista pana se gaseste resursa fileName
    while (node != NULL) {
        if (strcmp(node->info->name, fileName) == 0) {
            if (node->info->type == 1) {
                // daca este folder, se afiseaza eroarea
                printf("rm: cannot remove '%s': Is a directory\n", fileName);
                return;
            }
            if (prev == NULL) {
                // daca resursa este head-ul listei, atunci
                // head-ul devine urmatoarea resursa
                cnt->children->head = node->next;
                // se elibereaza memoria
                free_TreeNode(node->info);
                free(node);
            } else {
                // daca resursa nu este head, atunci resursa anterioara
                // va indica catre resursa posterioara resursei curente
                prev->next = node->next;
                // se elibereaza memoria
                free_TreeNode(node->info);
                free(node);
            }
            return;
        }
        prev = node;
        node = node->next;
    }
    // daca nu exista resursa fileName, se afiseaza eroarea
    printf("rm: failed to remove '%s': No such file or directory\n", fileName);
}


void rmdir(TreeNode* currentNode, char* folderName) {
    FolderContent *cnt = currentNode->content;
    ListNode *node = cnt->children->head;
    ListNode *prev = NULL;

    // se parcurge lista pana se gaseste resursa folderName
    while (node != NULL) {
        if (strcmp(node->info->name, folderName) == 0) {
            if (node->info->type == 0) {
                // daca este de tip file, se afiseaza eroarea
                printf("rmdir: failed to remove '%s': ", folderName);
                printf("Not a directory\n");
                return;
            }
            FolderContent *cnt2 = node->info->content;
            if (cnt2->children->head != NULL) {
                // daca directorul nu este gol, se afiseaza eroarea
                printf("rmdir: failed to remove '%s': ", folderName);
                printf("Directory not empty\n");
                return;
            }
            if (prev == NULL) {
                // daca resursa este head-ul listei, atunci
                // head-ul devine urmatoarea resursa
                cnt->children->head = node->next;
                // se elibereaza memoria
                free_TreeNode(node->info);
                free(node);
            } else {
                // daca resursa nu este head, atunci resursa anterioara
                // va indica catre resursa posterioara resursei curente
                prev->next = node->next;
                // se elibereaza memoria
                free_TreeNode(node->info);
                free(node);
            }
            return;
        }
        prev = node;
        node = node->next;
    }
    // daca nu exista resursa folderName, se afiseaza eroarea
    printf("rmdir: failed to remove '%s': ", folderName);
    printf("No such file or directory\n");
}


void touch(TreeNode* currentNode, char* fileName, char* fileContent) {
    FolderContent *cnt = currentNode->content;

    if (cnt->children->head == NULL) {
        // daca lista de resurse a directorului curent este goala,
        // atunci fisierul care se creeaza este head-ul
        cnt->children->head = malloc(sizeof(struct ListNode));
        unsigned int type = 0;
        cnt->children->head->info =
            create_TreeNode(currentNode, fileName, type, fileContent);
        cnt->children->head->next = NULL;
        return;
    }
    // fisierul care se creeaza este adaugat la sfarsitul listei
    ListNode *node = cnt->children->head;

    while (node->next != NULL) {
        // se parcurge lista si se verifica daca exista deja
        // un fisier/director cu acelasi nume
        if (strcmp(node->info->name, fileName) == 0)
            return;
        node = node->next;
    }
    if (strcmp(node->info->name, fileName) == 0)
        return;
    node->next = malloc(sizeof(struct ListNode));
    unsigned int type = 0;
    node->next->info =
        create_TreeNode(currentNode, fileName, type, fileContent);
    node->next->next = NULL;
}


void cp(TreeNode* currentNode, char* source, char* destination) {
    TreeNode* dest = currentNode;
    // se creeaza o copie pentru calea catre sursa
    char *copy = malloc(strlen(source) + 1);
    strcpy(copy, source);
    FolderContent *cnt = currentNode->content;
    ListNode *node = cnt->children->head;

    // se parcurge calea
    char *p = strtok(source, "/");
    while (p != NULL) {
        // daca p e .. se face referire la directorul parinte
        if (strcmp(p, "..") == 0) {
            if (currentNode->parent != NULL) {
                currentNode = currentNode->parent;
                cnt = currentNode->content;
                node = cnt->children->head;
            }
        } else {
            // se parcurg resursele directorului curent
            while (node != NULL) {
                if (strcmp(node->info->name, p) == 0) {
                    // daca se gaseste resursa cu numele p
                    // atunci se avanseaza in ierarhia de fisiere
                    currentNode = node->info;
                    cnt = currentNode->content;
                    node = cnt->children->head;
                    break;
                }
                node = node->next;
            }
        }
        p = strtok(NULL, "/");
    }
    if (currentNode->type == 1) {
        // daca calea sursa este a unui director,
        // atunci se afiseaza eroarea
        printf("cp: -r not specified; omitting directory '%s'\n", copy);
        free(copy);
        return;
    }
    free(copy);
    // currentNode contine fisierul de la calea sursa

    // se creeaza o copie pentru calea catre destinatie
    copy = malloc(strlen(destination) + 1);
    strcpy(copy, destination);
    cnt = dest->content;
    node = cnt->children->head;

    // dest va contine fisierul/directorul de la calea destinatie
    // se parcurge calea
    p = strtok(destination, "/");
    char *prev = NULL;
    while (p != NULL) {
        // daca prev e .. se face referire la directorul parinte
        if (prev != NULL && strcmp(prev, "..") == 0) {
            if (dest->parent != NULL) {
                dest = dest->parent;
                cnt = dest->content;
                node = cnt->children->head;
            } else {
                // daca nu exista director parinte, atunci se
                // afiseaza eroarea
                printf("cp: failed to access '%s': Not a directory\n", copy);
                free(copy);
                return;
            }
        } else {
            int ok = 0;
            if (prev == NULL)
                ok = 1;

            // se parcurg resursele directorului curent
            while (node != NULL && prev != NULL) {
                if (strcmp(node->info->name, prev) == 0 &&
                    node->info->type == 1) {
                    // daca se gaseste directorul cu numele prev
                    // atunci se avanseaza in ierarhia de fisiere
                    dest = node->info;
                    cnt = dest->content;
                    node = cnt->children->head;
                    ok = 1;
                    break;
                }
                node = node->next;
            }

            // daca nu exista directorul prev prin resursele directorului
            // curent, atunci se afiseaza eroarea
            if (ok == 0) {
                printf("cp: failed to access '%s': Not a directory\n", copy);
                free(copy);
                return;
            }
        }
        prev = p;
        p = strtok(NULL, "/");
    }
    free(copy);

    if (strcmp(prev, "..") == 0) {
        // cazul in care destinatia este un director
        dest = dest->parent;
        FileContent *content = currentNode->content;
        touch(dest, mystrdup(currentNode->name), mystrdup(content->text));
        return;
    }

    while (node != NULL) {
        if (strcmp(node->info->name, prev) == 0 && node->info->type == 1) {
            // cazul in care destinatia este un director
            dest = node->info;
            FileContent *content = currentNode->content;
            touch(dest, mystrdup(currentNode->name), mystrdup(content->text));
            return;
        } else if (strcmp(node->info->name, prev) == 0 &&
                node->info->type == 0) {
            // cazul in care destinatia este un fisier existent
            dest = node->info;
            FileContent *content = currentNode->content;
            FileContent *content2 = dest->content;
            free(content2->text);
            content2->text = mystrdup(content->text);
            return;
        }
        node = node->next;
    }
    // cazul in care destinatia este un fisier care nu exista
    FileContent *content = currentNode->content;
    touch(dest, mystrdup(currentNode->name), mystrdup(content->text));
}

// functie care muta un folder si toate resursele sale
void mv_folder(char *prev, TreeNode *dest, TreeNode *currentNode,
            ListNode *node) {
    // daca prev este .. se face referire la directorul parinte
    if (prev != NULL && strcmp(prev, "..") == 0) {
        dest = dest->parent;
        // se creeaza in destinatie directorul ce va fi mutat
        mkdir(dest, mystrdup(currentNode->name));

        // se muta toate resursele directorului
        FolderContent *content = dest->content;
        ListNode *tmp = content->children->head;
        while (strcmp(tmp->info->name, currentNode->name) != 0) {
            tmp = tmp->next;
        }

        dest = tmp->info;
        // se parcurge lista de resurse
        content = currentNode->content;
        ListNode *tmp_curr = content->children->head;
        while (tmp_curr != NULL) {
            if (tmp_curr->info->type == 0) {
                FileContent *file = tmp_curr->info->content;
                touch(dest, mystrdup(tmp_curr->info->name),
                    mystrdup(file->text));
            } else {
                mv_folder(NULL, dest, tmp_curr->info, tmp_curr);
            }
            tmp_curr = tmp_curr->next;
        }
        // se sterge directorul (si toate resursele sale) de la sursa
        rmrec(currentNode->parent, currentNode->name);
        return;
    }

    while (node != NULL) {
        // se parcurg resursele directorului curent
        if (prev != NULL && strcmp(node->info->name, prev) == 0 &&
            node->info->type == 1) {
            // daca se gaseste directorul cu numele prev
            // atunci se avanseaza in ierarhia de fisiere
            dest = node->info;
            // se creeaza in destinatie directorul ce va fi mutat
            mkdir(dest, mystrdup(currentNode->name));

            // se muta toate resursele directorului
            FolderContent *content = dest->content;
            ListNode *tmp = content->children->head;
            while (strcmp(tmp->info->name, currentNode->name) != 0) {
                tmp = tmp->next;
            }

            dest = tmp->info;
            // se parcurge lista de resurse
            content = currentNode->content;
            ListNode *tmp_curr = content->children->head;
            while (tmp_curr != NULL) {
                if (tmp_curr->info->type == 0) {
                    FileContent *file = tmp_curr->info->content;
                    touch(dest, mystrdup(tmp_curr->info->name),
                        mystrdup(file->text));
                } else {
                    mv_folder(NULL, dest, tmp_curr->info, tmp_curr);
                }
                tmp_curr = tmp_curr->next;
            }
            // se sterge directorul (si toate resursele sale) de la sursa
            rmrec(currentNode->parent, currentNode->name);
            return;
        }
        node = node->next;
    }

    // prev este NULL - atunci functia este apelata
    // pentru ca directorul care este mutat contine alte directoare

    // se creeaza in destinatie directorul ce va fi mutat
    mkdir(dest, mystrdup(currentNode->name));

    // se muta toate resursele directorului
    FolderContent *content = dest->content;
    ListNode *tmp = content->children->head;
    while (strcmp(tmp->info->name, currentNode->name) != 0) {
        tmp = tmp->next;
    }

    dest = tmp->info;
    // se parcurge lista de resurse
    content = currentNode->content;
    ListNode *tmp_curr = content->children->head;
    while (tmp_curr != NULL) {
        if (tmp_curr->info->type == 0) {
            FileContent *file = tmp_curr->info->content;
            touch(dest, mystrdup(tmp_curr->info->name),
                mystrdup(file->text));
        } else {
            mv_folder(NULL, dest, tmp_curr->info, tmp_curr);
        }
        tmp_curr = tmp_curr->next;
    }
}

// functie care muta un fisier
void mv_file(char *prev, TreeNode *dest, TreeNode *currentNode,
        ListNode *node) {
    if (strcmp(prev, "..") == 0) {
        // cazul in care destinatia este un director
        dest = dest->parent;
        FileContent *content = currentNode->content;
        touch(dest, mystrdup(currentNode->name), mystrdup(content->text));
        // se sterge fisierul de la sursa
        rm(currentNode->parent, currentNode->name);
        return;
    }

    while (node != NULL) {
        if (strcmp(node->info->name, prev) == 0 && node->info->type == 1) {
            // cazul in care destinatia este un director
            dest = node->info;
            FileContent *content = currentNode->content;
            touch(dest, mystrdup(currentNode->name), mystrdup(content->text));
            // se sterge fisierul de la sursa
            rm(currentNode->parent, currentNode->name);
            return;
        } else if (strcmp(node->info->name, prev) == 0 &&
                node->info->type == 0) {
            dest = node->info;
            // cazul in care destinatia este un fisier existent
            FileContent *content = currentNode->content;
            FileContent *content2 = dest->content;
            free(content2->text);
            content2->text = mystrdup(content->text);
            // se sterge fisierul de la sursa
            rm(currentNode->parent, currentNode->name);
            return;
        }
        node = node->next;
    }
    // cazul in care destinatia este un fisier care nu exista
    FileContent *content = currentNode->content;
    touch(dest, mystrdup(currentNode->name), mystrdup(content->text));
    // se sterge fisierul de la sursa
    rm(currentNode->parent, currentNode->name);
}

void mv(TreeNode* currentNode, char* source, char* destination) {
    TreeNode* dest = currentNode;
    // se creeaza o copie pentru calea catre sursa
    char *copy = malloc(strlen(source) + 1);
    strcpy(copy, source);
    FolderContent *cnt = currentNode->content;
    ListNode *node = cnt->children->head;

    // se parcurge calea
    char *p = strtok(source, "/");
    while (p != NULL) {
        // daca p e .. se face referire la directorul parinte
        if (strcmp(p, "..") == 0) {
            if (currentNode->parent != NULL) {
                currentNode = currentNode->parent;
                cnt = currentNode->content;
                node = cnt->children->head;
            }
        } else {
            // se parcurg resursele directorului curent
            while (node != NULL) {
                if (strcmp(node->info->name, p) == 0) {
                    // daca se gaseste resursa cu numele p
                    // atunci se avanseaza in ierarhia de fisiere
                    currentNode = node->info;
                    cnt = currentNode->content;
                    node = cnt->children->head;
                    break;
                }
                node = node->next;
            }
        }
        p = strtok(NULL, "/");
    }
    free(copy);
    // currentNode contine resursa de la calea sursa

    // se creeaza o copie pentru calea catre destinatie
    copy = malloc(strlen(destination) + 1);
    strcpy(copy, destination);
    cnt = dest->content;
    node = cnt->children->head;

    // dest va contine resursa de la calea destinatie
    // se parcurge calea
    p = strtok(destination, "/");
    char *prev = NULL;
    while (p != NULL) {
        // daca prev e .. se face referire la directorul parinte
        if (prev != NULL && strcmp(prev, "..") == 0) {
            if (dest->parent != NULL) {
                dest = dest->parent;
                cnt = dest->content;
                node = cnt->children->head;
            } else {
                // daca nu exista director parinte, atunci se
                // afiseaza eroarea
                printf("mv: failed to access '%s': Not a directory\n", copy);
                free(copy);
                return;
            }
        } else {
            int ok = 0;
            if (prev == NULL)
                ok = 1;

            // se parcurg resursele directorului curent
            while (node != NULL && prev != NULL) {
                if (strcmp(node->info->name, prev) == 0 &&
                    node->info->type == 1) {
                    // daca se gaseste directorul cu numele prev
                    // atunci se avanseaza in ierarhia de fisiere
                    dest = node->info;
                    cnt = dest->content;
                    node = cnt->children->head;
                    ok = 1;
                    break;
                }
                node = node->next;
            }

            // daca nu exista directorul prev prin resursele directorului
            // curent, atunci se afiseaza eroarea
            if (ok == 0) {
                printf("mv: failed to access '%s': Not a directory\n", copy);
                free(copy);
                return;
            }
        }
        prev = p;
        p = strtok(NULL, "/");
    }
    free(copy);

    if (currentNode->type == 0) {
        // cazul in care se muta un fisier
        mv_file(prev, dest, currentNode, node);
    } else {
        // cazul in care se muta un folder
        mv_folder(prev, dest, currentNode, node);
    }
}

