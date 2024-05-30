#include "config.h"
#include <cef_app.h>
#include <cef_client.h>
#include <cef_render_handler.h>

int main(int argc, char* argv[]) {
	int pid = getpid();
	printf("Start cef subprocess [%d]:\n", pid);
	for (int i=0; i<argc; ++i)
		printf("  - [%d] %s\n", pid, argv[i]);
	
	#ifdef TARGET_SYSTEM_IS_WINDOWS
	CefMainArgs main_args(GetModuleHandle(NULL));
	#else
	CefMainArgs main_args(argc, argv);
	#endif
	
	return CefExecuteProcess(main_args, nullptr, 0);
}
