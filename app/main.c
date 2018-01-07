#include <crank-vm/crank-vm.h>
#include <stdio.h>
#include <string.h>

static crankvm_context_t *context = NULL;
static const char *imageFileName = NULL;

static void printHelp(void)
{
    printf("TODO: printHelp()\n");
}

static void printVersion(void)
{
    printf("TODO: printVersion()\n");
}

int main(int argc, const char* argv[])
{
    // Parse the command line arguments.
    for(int i = 1; i < argc; ++i)
    {
        if(*argv[i] == '-')
        {
            if(!strcmp(argv[i], "-help"))
            {
                printHelp();
                return 0;
            }
            else if(!strcmp(argv[i], "-version"))
            {
                printVersion();
                return 0;
            }
            else
            {
                fprintf(stderr, "Unsupported argument %s\n", argv[i]);
                return 1;
            }
        }
        else
        {
            imageFileName = argv[i];
        }
    }

    // Validate the command line arguments.
    if(!imageFileName)
    {
        printHelp();
        return 1;
    }

    crankvm_error_t error = crankvm_context_create(&context);
    if(error)
    {
        fprintf(stderr, "Failed to create crank vm context: %s\n", crankvm_error_getString(error));
        return 0;
    }

    error = crankvm_context_loadImageFromFileNamed(context, imageFileName);
    if(error)
    {
        fprintf(stderr, "Failed to load image into crank vm context: %s\n", crankvm_error_getString(error));
        return 0;

    }

    error = crankvm_context_run(context);
    if(error)
    {
        fprintf(stderr, "Failed to run the image in a crank vm context: %s\n", crankvm_error_getString(error));
        return 0;

    }

    return 0;
}
