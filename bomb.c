#include <dirent.h>
#include <zip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int filter_docx(const struct dirent *);

void main() {
	struct dirent **files;
	int entry; 
	int i;

	entry = 0;
	i = 0;

	entry = scandir(".", &files, filter_docx, alphasort);
	if (entry < 0){
		perror("scandir");
	} else {
		for(i; i < entry; i++){
			printf("%s\n", files[i]->d_name);
			free(files[i]);
		}
		free(files);
	}
	printf("%d", entry);
}      

int filter_docx(const struct dirent *entry){
	const char *name_f = entry->d_name;
	size_t len = strlen(name_f);
	return (len > 5 && strcmp(name_f + len - 5, ".docx") == 0);
}



