#include <asm-generic/errno-base.h>
#include <linux/limits.h>
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h> // mkdir
#include <errno.h> // get libc error


enum ProgramError {
	LANGUAGE_ERR = 1,
	DIR_ERR,
	NO_FILE_ERR,
	MISMATCH_ERR,
	RESPONSE_ERR,
	USER_CANCEL,
	OUTPUT_MKDIR_ERR,
	SNPRINTF_ERR,
	SPRINTF_ERR,
	ARG_COUNT_ERR,
	THREAD_CREATE_ERR,
	THREAD_JOIN_ERR,
};

char* program_error_msg(int err) {
	switch (err) {
		case LANGUAGE_ERR:
			return "LANGUAGE_ERR: language not supported.";
		case DIR_ERR:
			return "DIR_ERR: that is not a directory.";
		case NO_FILE_ERR:
			return "NO_FILE_ERR: either there are no video files or sub files that meet the file format specified.";
		case MISMATCH_ERR:
			return "MISMATCH_ERR: no equal ammount of video and sub files.";
		case RESPONSE_ERR:
			return "RESPONSE_ERR: response was too long?";
		case USER_CANCEL:
			return "user cancelled execution.";
		case OUTPUT_MKDIR_ERR:
			return "OUTPUT_MKDIR_ERR: mkdir error out unexepectedly. Check permissios on the directory.";
		case SNPRINTF_ERR:
			return "SNPRINTF_ERR: encoding failed or command line string too long.";
		case SPRINTF_ERR:
			return "SPRINTF_ERR: sprintf encoding failed";
		case ARG_COUNT_ERR:
			return "ARG_COUNT_ERR: wrong amount of arguments.";
		case THREAD_CREATE_ERR:
			return "THREAD_CREATE_ERR: thread create failed.";
		case THREAD_JOIN_ERR:
			return "THREAD_JOIN_ERR: pthread join failed.";
		default:
			return "undefined error";
	}
}

int program_error_puts_return(int err) {
	puts(program_error_msg(err));
	return err;
}


typedef char* string;

#define MAX_PATH 4096



string langs(const string lang) {
	if (strcmp(lang, "jpn") == 0) {
		return "Japanese";
	} else {
		return NULL;
	}
}

int compare_str(const void *a, const void *b) {
	return strcmp((string)a, (string)b);
}

void* run_cmd(void* input) {
	//int ret = system((string)input);
	return NULL;
}



typedef struct {
	char arr[1000][PATH_MAX];
	size_t len;
} array;

array init_arr() {
	array ret = {
		.len = 0,
	};
	return ret;
}


void append_str(array* a, const char* input) {
	strcpy(a->arr[a->len], input);
	a->len += 1;
}

// TODO testing
int addsubs(const string dir, const string videoformat, const string subformat, const string lang) {
	
	string language = langs(lang);
	if (language == NULL) {
		return program_error_puts_return(LANGUAGE_ERR);
	}

	array videofiles = init_arr();
	array subfiles = init_arr();

	DIR *folder = opendir(dir);
	if (folder == NULL) {
		return program_error_puts_return(DIR_ERR);
	}

	struct dirent *entry;
	while ((entry = readdir(folder))) {
		if (entry->d_type == DT_REG) {
			if (strstr(entry->d_name, videoformat) != NULL) {
				append_str(&videofiles, entry->d_name);
			} else if (strstr(entry->d_name, subformat) != NULL) {
				append_str(&videofiles, entry->d_name);
			}
		}
	}
	closedir(folder);

	if (videofiles.len == 0 || subfiles.len == 0) {
		return program_error_puts_return(NO_FILE_ERR);
	}
	if (videofiles.len != subfiles.len) {
		return program_error_puts_return(MISMATCH_ERR);
	}

	qsort(videofiles.arr, videofiles.len, PATH_MAX, compare_str);
	qsort(subfiles.arr, subfiles.len, PATH_MAX, compare_str);

	puts("Are these pairs correct? (Y/n):");
	char response[10];
	if (scanf("%s", response) < 0) {
		return program_error_puts_return(RESPONSE_ERR);
	}
	if (strstr(response, "n") != NULL) {
		return program_error_puts_return(USER_CANCEL);
	}

	const string os = "/output";
	char output_dir[MAX_PATH];
	strcpy(output_dir, dir);
	strcat(output_dir, os);
	int ret = mkdir(output_dir, 0770);
	if (ret != 0 && errno != EEXIST) {
		return program_error_puts_return(OUTPUT_MKDIR_ERR);
	}

	// TODO test loop and join
	pthread_t thread_id;
	for (int i = 0; i < subfiles.len; i++) {
		//string sub = subfiles.ptr[i];
		string vid = videofiles.arr[i];

		const string mkvmerge_cmd_str = "mkvmerge -o %s/%s %s --language 0:%s --track-name 0:%s";
		string cmd = malloc(MAX_PATH*3 + strlen(mkvmerge_cmd_str)  + strlen(lang) + strlen(language) + 3);
		int ret = sprintf(cmd, mkvmerge_cmd_str, output_dir, vid, vid, lang, language);
		if (ret < 0) {
			return program_error_puts_return(SPRINTF_ERR);
		}
		
		ret = pthread_create(&thread_id, NULL, run_cmd, (void*)cmd);
		if (ret != 0) {
			return program_error_puts_return(THREAD_CREATE_ERR);
		}
		free(cmd);	
	}
	ret = pthread_join(thread_id,NULL);
	if (ret != 0) {
		return program_error_puts_return(THREAD_JOIN_ERR);
	}
	return 0;
}



int main(int argc, string argv[]) { 
	if (argc != 5) {
		return program_error_puts_return(ARG_COUNT_ERR);
	}
	int ret = addsubs(argv[1], argv[2], argv[3], argv[4]);
	if (ret != 0) {
		return ret;
	}

	return 0;
}
