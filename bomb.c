#include <dirent.h>
#include <zip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int filter_docx(const struct dirent *);
void extractor_txt(const char *, FILE *);
void extractor_imag(zip_t *, FILE *);

int main(int argc, char *argv[]) {
	const char *progname = argv[0];
	struct dirent **files;
	zip_t* docx;
	int entry; 
	int i;
	int err;
	
	entry = 0;
	i = 0;
	err = 0;

	entry = scandir(".", &files, filter_docx, alphasort);
	if (entry < 0){
		perror("scandir");
		return 1;
	}
	for(i; i < entry; i++){
		if ((docx = zip_open(files[i]->d_name, 0, &err)) != NULL){
			printf("%s{}{}Foi aberto com sucesso\n", files[i]->d_name);
			zip_file_t *xml_file = zip_fopen(docx, "word/document.xml", 0);
			if (xml_file == NULL) {
				printf("document.xml não encontrado");
				zip_close(docx);
			}
			
			zip_stat_t stats;
			if (zip_stat(docx, "word/document.xml", 0, &stats) != 0) {
				printf("Erro na obteção de dados sobre xml.");
				zip_fclose(xml_file);
				zip_close(docx);
			}

			char *xml_content = (char *)malloc(stats.size + 1);
			if(xml_content == NULL){
				printf("Houve um erro na alocação.");
				zip_fclose(xml_file);
				zip_close(docx);
			}
			zip_int64_t bytes_read = zip_fread(xml_file, xml_content, stats.size);
			xml_content[bytes_read] = '\0';
			
			char *output_name;
			asprintf(&output_name, "%s.txt", files[i]->d_name);

			FILE *txt_out = fopen(output_name, "w");
			free(output_name);

			if (!txt_out){
				printf("Erro ao criar o txt.out");
				zip_fclose(xml_file);
				zip_close(docx);
				return 1;
			}

			extractor_txt(xml_content, txt_out);
			extractor_imag(docx, txt_out);
			fclose(txt_out);
			free(xml_content);
			zip_fclose(xml_file);
			zip_close(docx);
		} else{
			zip_error_t error;
			zip_error_init_with_code(&error, err);
			fprintf(stderr, "%s: cannot open docx archive '%s': %s\n",
					progname, files[i]->d_name, zip_error_strerror(&error));
			zip_error_fini(&error);
		}
			free(files[i]);	

	}
	free(files);
	return 0;
	
}      

int filter_docx(const struct dirent *entry){
	const char *name_f = entry->d_name;
	size_t len = strlen(name_f);
	return (len > 5 && strcmp(name_f + len - 5, ".docx") == 0);
}

void extractor_txt(const char *document_xml, FILE *out_txt) {
	const char *tag_start = "<w:t>";
	const char *tag_end = "</w:t>";
	const char *ptr = document_xml;

	while ((ptr = strstr(ptr, tag_start)) != NULL) {
		ptr = strchr(ptr, '>') + 1;
		const char *end = strstr(ptr, tag_end);
		if (!end) break;
		
		fwrite(ptr, 1, end - ptr, out_txt);
		fputc(' ', out_txt);
		ptr = end + strlen(tag_end);
	}

}

void extractor_imag(zip_t *docx, FILE *out_txt) {
	zip_t *pdocx = docx;
	int i;
	i = 0;
	zip_int64_t num_entrie = zip_get_num_entries(pdocx, 0);
	
	if (num_entrie < 0) {
		fprintf(stderr, "Erro ao tentar pegar o numero de entradas\n");
		return;
	}

	for (i; i < num_entrie; i++) {
		const char *name = zip_get_name(pdocx, i, 0);
		if (name && strstr(name, "word/media/") != NULL) {
			const char *filename = strrchr(name, '/');
			if(filename) filename++; else filename = name;
			fprintf(out_txt, "%s\n", filename);}
	}
}





