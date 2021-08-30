#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_LENGTH 128

typedef struct {
  char debug_mode; 
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;


} state;
struct menu {
    char *func_name;
    void (*fun)(state* s);
}menu;
void toggle_debug_mode(state * s) {
    if(s->debug_mode== '0' ){
        s->debug_mode='1';
        printf("Debug flag now on\n");
    }
    else{
        s->debug_mode='0';
        printf("Debug flag now off\n");
    }
}
void set_file_name(state * s) { 
    printf("Please enter file name:\n");
    fgets(s->file_name, 128, stdin);
    s->file_name[strlen(s->file_name)-1]='\0';
	if(s->debug_mode=='1') 
		printf("Debug: file name set to %s\n", s->file_name);
}

void set_unit_size(state * s) { 
    printf("Please enter unite size:\n");
    char input[MAX_LENGTH];
    fgets(input, 128, stdin);
    int unit_size=atoi(input);
    if (unit_size==1 || unit_size==2 || unit_size==4){
        s->unit_size=unit_size;
	    if(s->debug_mode=='1') {
		    printf("Debug: set size to %d\n", s->unit_size); 
        }
    }
    else
    {
        printf("Wrong value is entered!\n");
    }
    
}
void quit(state * s){ 

   
	if(s->debug_mode=='1') 
		printf("quitting\n");
    exit(0);

}
 

void printState(state * s){
    printf("unit_size: %d \nfile_name: %s mem_count: %d\n", s->unit_size,s->file_name,s->mem_count);

}
char* unit_to_format(int unit) {
    static char* formats[] = {"%hhd\t%hhX\n", "%hd\t%hX\n", "No such unit", "%d\t%X\n"};
    return formats[unit-1];
}
void mem_disp(state * s){
    char* tmp = (char*) s->mem_buf;
   char* end = tmp + s->unit_size*s->mem_count;
   fprintf(stdout,"Decimal\tHexadecimal\n=============\n");
    while (tmp < end) {
        int var = *((int*)(tmp));
        fprintf(stdout, unit_to_format(s->unit_size),var, var);
        tmp += s->unit_size;
    }
}
void fileModify(state* s){
     fprintf(stdout,"Please enter <location> <val>\n");
    char buf[128];
    int location;
    int val;
    
    fgets(buf, 128, stdin);
	sscanf(buf, "%x %x", &location, &val);
    FILE* file=fopen(s->file_name,"r+");
    fseek(file,location,SEEK_SET);
   
    fwrite(&val,s->unit_size,1,file);
    fclose(file);
}
void saveIntoFile(state* s){
    fprintf(stdout,"Please enter <source-address> <target-location> <length>\n");
    char buf[128];
    int source;
    int target;
    int length;
  
    fgets(buf, 128, stdin);
	sscanf(buf, "%x %x %d", &source, &target,&length);
    FILE* file=fopen(s->file_name,"r+");
    if(source==0){
        source=(int)s->mem_buf;
    }
    fseek(file,target,SEEK_SET);
    unsigned char *sourcePtr = (unsigned char *)source;
    fwrite(sourcePtr,s->unit_size,length,file);
    fclose(file);
}
void loadIntoMem(state * s){
    char buf[128];
    int loc;
    int length;
    printf("Please enter <location> <length>\n");
    fgets(buf, 128, stdin);
	sscanf(buf, "%x %d", &loc, &length);
    FILE* file=fopen(s->file_name,"r+");
    if(file==NULL){
        fprintf(stderr,"file don't exist\n");
        return;
    }
    fseek(file,loc,SEEK_SET);
    fread(s->mem_buf,s->unit_size,length,file);
    s->mem_count=length;
    fprintf(stdout,"Loaded %d units into memory",length);
    fclose(file);
}
int main(int argc, char **argv){
    char input [MAX_LENGTH];
    state * s=malloc (sizeof(state));
    s->unit_size = 1;
    struct menu menu[] = { { "Toggle Debug Mode",toggle_debug_mode}, { "Set File Name",set_file_name}, { "Set Unit Size",set_unit_size},{"Load Into Memory",loadIntoMem},{"Memory Display",mem_disp},{"Save Into File",saveIntoFile},{"File Modify",fileModify},{ "Quit", quit}, {NULL,NULL} };
    while(1){
        int i=0;
        printState(s);
        while(menu[i].func_name!=NULL){
            printf("%d) %s\n",i,menu[i].func_name);
            i++;
        }
        printf("Option: ");
        fgets(input, MAX_LENGTH, stdin);
        int idx= atoi(input);
        if (idx>=0 && idx<=i){
            printf("Within bounds\n");
            menu[idx].fun(s);
           
        }
        i=0;
    }
}