#include <stdio.h>
#include <string.h>
#include <stdlib.h>


 


typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    char sig[];
} virus;
typedef struct link link;
struct link {
    link *nextVirus;
    virus *vir;
};
typedef struct fun_desc {
  char *name;
  char (*fun)();
}fun_desc; 
link* vir_list=NULL;
link* list_append(link* virus_list, virus* data){
    link *tmplink = malloc(sizeof(link));
    if(virus_list!=NULL){
         
         tmplink->nextVirus=virus_list;
         tmplink->vir=data;
         return tmplink;
    }else{
        tmplink->nextVirus=NULL;
        tmplink->vir=data;
        return tmplink;
    }
}
void list_free(link *virus_list){
    if(virus_list->nextVirus!=NULL){
        list_free(virus_list->nextVirus);
    }
    free(virus_list->vir);
    free(virus_list);
}
void readVirus(){
    char path[256];
    char *n;
    fgets(path,256,stdin);
    unsigned short val;
    virus *v;

    n=strchr(path,'\n');
    *n='\0';
    printf("%s\n",path);
    FILE * input = fopen(path,"r");
    while(fread(&val,sizeof(unsigned short),1,input)>0){
    //fseek(input,-2,SEEK_CUR);

    v= (virus*) malloc(val-2);
    v->SigSize = val;
    fread(v->virusName,v->SigSize-2,1,input);
    v->SigSize -= sizeof(virus);
    vir_list = list_append(vir_list,v);
    }
    fclose(input);
    
}
    void detect_virus(char *buffer, unsigned int size){
    int cmp;
    link * tmpVirusList = vir_list;
    while(tmpVirusList!=NULL){
        for (int i = 0; i < size; i++) {
            cmp=memcmp(tmpVirusList->vir->sig,buffer+i,tmpVirusList->vir->SigSize);
            if(cmp==0){
                printf("virus location: %d\n",i);
                printf("virus name: %s\n",tmpVirusList->vir->virusName);
                printf("sig size: %d\n",tmpVirusList->vir->SigSize);

            }
        }
        tmpVirusList=tmpVirusList->nextVirus;
    }

    }
void list_print(){
    link * virus_list = vir_list;
    while(virus_list!=NULL){
        virus *v = virus_list->vir;
        printf("Virus name: %s\n",v->virusName);
        printf("Virus size: %d\n",v->SigSize);
        for(int i=0;i<v->SigSize;i++)
            printf("%hhX ",v->sig[i]);
        printf("\n");
        virus_list=virus_list->nextVirus;
    }
}
void detect() {
    char fileBuffer[10000];
    unsigned int fileSize;
    char path[256];
    char *n;
    
    fgets(path, 256, stdin);

    n=strchr(path,'\n');
    *n='\0';

    FILE *input = fopen(path, "r");
    //from stackOverFlow
    fseek(input, 0, SEEK_END); // seek to end of file
    fileSize = ftell(input); // get current file pointer
    fseek(input, 0, SEEK_SET); // seek back to beginning of file
    fread(fileBuffer,1,fileSize,input);

        detect_virus(fileBuffer, fileSize);
    fclose(input);

}
void quit(){
    list_free(vir_list);
    exit(0);
}
int main(int argc, char **argv)
{   
    int i=1;
    
    char buffer[100];
    int opt=0;
    struct fun_desc menu[] ={ {"Load signatures",readVirus},
                                {"Print signatures",list_print},
                                {"Detect viruses",detect},
                                {"Quit",quit},
                                {NULL,NULL}};
    while(1){
        i=1;
        while(menu[i-1].name!=NULL){
              printf("%d) %s\n",i,menu[i-1].name);
        	  i++;
        }
        fgets(buffer,100,stdin);
        opt = strtol(buffer,NULL,10);
        menu[opt-1].fun();
    }
}