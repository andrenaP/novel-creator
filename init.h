#pragma once

#ifdef __linux__
	#define OPEN(file_name, mode)   popen(file_name, mode)
	#define CLOSE(file)             pclose(file)
#else
	#define OPEN(file_name, mode)   fopen(file_name, mode)
	#define CLOSE(file)             fclose(file)
#endif