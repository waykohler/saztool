#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "miniz.c"
};

int main(int argc,char *argv[])
{
	if(argc != 4) {
		fprintf(stderr,"USAGE: %s extract <saz> <bin>\n",argv[0]);
		return 1;
	}

	const char *inPath = argv[2];
	const char *outPath = argv[3];

	mz_zip_archive zip={0};
	if(!mz_zip_reader_init_file(&zip,inPath,MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY)) {
		fprintf(stderr,"Failed to open '%s' for reading or invalid archive.\n",inPath);
		return 1;
	}

	FILE *outFile = fopen(outPath,"wb");
	if(outFile == NULL) {
		fprintf(stderr,"Failed to open '%s' for writing.\n",outPath);
		return 1;
	} 

	int files = mz_zip_reader_get_num_files(&zip);
	for(int i=0;i<files;i++) {
		char filename[1024];
		if(!mz_zip_reader_get_filename(&zip,i,filename,sizeof(filename))) {
			continue;
		}
		int httpIdx = -1;
		char ext[1024];
		if(sscanf(filename,"raw/%d_c.%s",&httpIdx,ext) != 2) {
			continue;
		}
		
		size_t postSize;
		char *post = (char *)mz_zip_reader_extract_to_heap(&zip,i,&postSize,0);
		if(post == NULL) {
			continue;
		}
		char *data = NULL;
		int nl=0;
		for(size_t j=0;j<postSize;j++) {
			if(post[j] == '\n' || post[j] == '\r') {
				nl++;
				if(nl == 4) {
					data = post+j+1;
					break;
				}
			} else {
				nl = 0;
			}
		}
		if(data && postSize-(data-post)) {
			fwrite(data,1,postSize-(data-post),outFile);
		}
		printf("%s\n",filename);
		free(post);
	}

	fclose(outFile);
	return 0;
}