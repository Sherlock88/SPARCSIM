#include <simulator.h>



/*
 * Execution begins here.
 */
int main(int argc, char* argv[])
{
	char simulatorCommand[MAX_INPUT_LENGTH];
	
        // Initialize memory, registers, breakpoint list.
	initializeMemory();
        initializeRegisters();
	initializeBreakPointList();
	
        // Check whether any command-line parameter has been passed.
	if(argc == 3)
	{
                // Call for disassembly, '-d' switch has been passed.
		if(!(strcmp(argv[1], "-d") && strcmp(argv[1], "--dis")))
		{
			struct loadedSections *elfSectionsPrevPtr, *elfSectionCurPtr;

			elfSectionCurPtr = load_sparc_instructions(argv[2]);

			sprintf(simulatorCommand, "load %s", argv[2]);          // Form a 'load' instruction.
			printf("\n");
			processSimulatorCommand(simulatorCommand);
			printf("\n");

                        // Iterate over all ELF sections.
			do
			{
				printf("Disassembly of Section: %s\n\n", elfSectionCurPtr->sectionName);
				sprintf(simulatorCommand, "dis %lu %lu", elfSectionCurPtr->sectionLoadAddress, elfSectionCurPtr->instructionCount);     // Form a 'dis' instruction.
				processSimulatorCommand(simulatorCommand);
				elfSectionsPrevPtr = elfSectionCurPtr;
				free(elfSectionsPrevPtr);
				elfSectionCurPtr = elfSectionCurPtr->nextSection;
			}
			while(elfSectionCurPtr != NULL);
			printf("\n");
			return RET_SUCCESS;
		}

	}

        // If command-line parameter is not passed, display usual welcome banner and drop user to an interactive shell. 
	printf("\nSPARC v8 Simulator\n");
	printf("******************\n");
	printf("System Configuration: RAM = 4GB\n\n");

        // Has a file name been passed to load into simulator memory?
	if(argc == 2)
	{
		sprintf(simulatorCommand, "load %s", argv[1]);                  // Form a 'load' command.
		processSimulatorCommand(simulatorCommand);
	}
	
        // Call for batch processing, '-b' switch has been passed.
	if(argc == 3)
	{
		if(!(strcmp(argv[1], "-c") && strcmp(argv[1], "--batch")))
		{
			sprintf(simulatorCommand, "batch %s", argv[2]);         // Form a 'batch' command.
			if(processSimulatorCommand(simulatorCommand) == RET_QUIT)
				return RET_SUCCESS;
		}
	}

        // Run into a loop until 'quit' command is entered.
	while(1)
	{
		printf("sparcsim> ");
		fgets(simulatorCommand, MAX_INPUT_LENGTH, stdin);
		if(processSimulatorCommand(simulatorCommand) == RET_QUIT)
			return RET_SUCCESS;
	}
}



/* 
 * Processes the commands issued to the simulator.
 */
int processSimulatorCommand(char* simulatorCommand)
{
	char* command = NULL, *firstParametre = NULL, *secondParametre = NULL, *arguments = (char*)malloc(200);
	unsigned long firstNumericParametre = 0, secondNumericParametre = 0;
        unsigned short count = 0;
        static short isVerbose = 0;             // Verbocity has to be set back to default by the calling command;
	const char delimiters[] = " \n\t";
	char* hexNumber = (char*)malloc(32);
        struct watchPointInfo* watchInfo;
	
        // Strip off #
        for(count = 0; count < strlen(simulatorCommand); count++)
            if(simulatorCommand[count] == '#')
                simulatorCommand[count] = '\0';
        
	strcpy(arguments, simulatorCommand);
        
        // Split the command to individual tokens separated by whitespaces.
	command = strtok(simulatorCommand, delimiters);
	if(!command)
		return RET_NOTACOMMAND;
        
        // Extract the first parameter and convert it to unsigned integer.
	firstParametre = strtok(NULL, delimiters);
	if(firstParametre != NULL)
	{
		secondParametre = strtok(NULL, delimiters);
		firstNumericParametre = strtoul(firstParametre, NULL, 0);
        }
        
        // Extract the second parameter and convert it to unsigned integer.
	if(secondParametre != NULL)
	{
		while(strtok(NULL, delimiters));
		secondNumericParametre = strtoul(secondParametre, NULL, 0);
        }
	
	
	// Quit
	if(!(strcmp(command, "quit") && strcmp(command, "q")))
		return RET_QUIT;
	
	
	// help
	if(!(strcmp(command, "help") && strcmp(command, "h")))
	{
		printf("\n\tsparcsim  [file_name]       |  load a file into simulator memory\n");
		printf("\tsparcsim -d [file_name]     |  disassemble SPARC ELF binary\n");
		printf("\tsparcsim -c [file_name]     |  execute a batch file of SPARCSIM commands\n");
		printf("\t[ba]tch <file>              |  execute a batch file of SPARCSIM commands\n");
		printf("\t[re]set                     |  reset simulator \n");
		printf("\t[l]oad <file_name>          |  load a file into simulator memory\n");
		printf("\t[m]em [addr] [count]        |  display memory at [addr] for [count] bytes\n");
		printf("\t[w]mem <addr> <val>         |  write memory word at <addr> with value <val>\n");
		printf("\t[s]tep                      |  single step\n");
                printf("\t[t]ra [inst_count]          |  trace [inst_count] instructions\n");
		printf("\t[br]eak <addr>              |  add a breakpoint at <addr>\n");
		printf("\t[de]l <num>                 |  delete breakpoint <num>\n");
		printf("\t[br]eak                     |  print all breakpoints\n");
		printf("\t[r]eg [reg] [val]           |  show/set integer registers (or windows, eg 'reg w2'\n");
                printf("\t[p]sr                       |  show PSR fields\n");
                printf("\t[fs]r                       |  show FSR fields\n");
                printf("\t[tb]r                       |  show TBR fields\n");
                printf("\t[f]loat                     |  print the FPU registers\n");
		printf("\t[d]is [addr] [count]        |  disassemble [count] instructions at address [addr]\n");
		printf("\t[c]ont [cnt]                |  continue execution for [cnt] instructions\n");
                printf("\t[wa]tch <addr]>             |  add a watchpoint at <addr>\n");
		printf("\t[g]o <addr> [cnt]           |  start execution at <addr>\n");
		printf("\t[ru]n [cnt]                 |  reset and start execution at address zero\n");
		printf("\t[h]elp                      |  display this help\n");
		printf("\t[e]cho <string>             |  print <string> to the simulator window\n");
		printf("\t[sh]ell <cmd>               |  execute shell command\n");
		printf("\t[q]uit                      |  exit the simulator\n\n");

		return RET_SUCCESS;
	}
	
	
	// [re]set
	if(!(strcmp(command, "reset") && strcmp(command, "re")))
	{
		resetSimulator();
		return RET_SUCCESS;
	}


	// [ba]tch
	if(!(strcmp(command, "batch") && strcmp(command, "ba")))
	{
            if(firstParametre == NULL)
			return RET_FAILURE;
		else
		{
			FILE* handle = fopen(firstParametre, "r");
			if(!handle)
			{
				printf("Batch file: %s does not exist\n", firstParametre);
				return RET_FAILURE;
			}
			
			char* buffer = (char*)malloc(150);
			
			while(fgets(buffer, 150, handle))
			{
				short bufferIndex = -1;
                                char* trimmedCommand;
				
				// Strip off trailing carriage return, newline
				while(buffer[++bufferIndex] != '\0' && buffer[bufferIndex] != '\r' && buffer[bufferIndex] != '\n');
				buffer[bufferIndex] = '\0';
				
				if(!buffer || !strlen(buffer))
                                        continue;

                                trimmedCommand = trim(buffer);
                                if(strlen(trimmedCommand))
                                        printf("sparcsim> %s\n", trimmedCommand);
                                        
				switch(processSimulatorCommand(buffer))
				{
                                        case RET_FAILURE: printf("          Error executing command above\n"); return RET_FAILURE;
					case RET_QUIT: return RET_QUIT;
				}					
			}		
			
			free(buffer);
			fclose(handle);
			return RET_SUCCESS;
		}
	}
	
	
	// [l]oad
	if(!(strcmp(command, "load") && strcmp(command, "l")))
	{
		struct loadedSections *elfSectionsPrevPtr, *elfSectionCurPtr;

		elfSectionCurPtr = load_sparc_instructions(firstParametre);
		printf("\n");

		switch(elfSectionCurPtr->sectionType)
		{
		case ELF_FILE_DOES_NOT_EXIST_ERROR:
			printf("Couldn't open: %s\n", firstParametre);
			return RET_FAILURE;
		case ELF_BINARY_OUT_OF_DATE:
			printf("Warning: ELF library is out of date\n");
			return RET_FAILURE;
		case ELF_POINTER_INITIALIZATION_ERROR:
			printf("Couldn't initialize ELF pointer\n");
			return RET_FAILURE;
		case ELF_SECTION_LOAD_ERROR:
			printf("Couldn't load ELF sections\n");
			return RET_FAILURE;
		}

                // Iterate over all ELF sections.
		do
		{
			printf("Loaded Section: %s, Address: 0x%lx, Size: %lu, Type: ", elfSectionCurPtr->sectionName, elfSectionCurPtr->sectionLoadAddress, elfSectionCurPtr->sectionSize);
			switch(elfSectionCurPtr->sectionType)
			{
				case CODE_SECTION: printf("Code, Instructions: %lu\n", elfSectionCurPtr->instructionCount); break;
				case DATA_SECTION: printf("Data\n"); break;
				case UNINITIALIZED_DATA_SECTION: printf("Uninitialized Data\n"); break;
			}
			elfSectionsPrevPtr = elfSectionCurPtr;
			free(elfSectionsPrevPtr);
			elfSectionCurPtr = elfSectionCurPtr->nextSection;
		}
		while(elfSectionCurPtr != NULL);
		printf("\n");
		
		return RET_SUCCESS;
	}
	
	
	// [m]em
	if(!(strcmp(command, "mem") && strcmp(command, "m")))
	{
		if(firstParametre == NULL && secondParametre == NULL)
			return RET_FAILURE;
		else
		{
			if(secondParametre == NULL)
				displayMemoryArea(firstNumericParametre, 64);
			else
				displayMemoryArea(firstNumericParametre, secondNumericParametre);
				
			printf("\n\n");
			return RET_SUCCESS;
		}
	}
	
	
	// [w]mem
	if(!(strcmp(command, "wmem") && strcmp(command, "w")))
	{
		if(firstParametre == NULL && secondParametre == NULL)
			return RET_FAILURE;
		else
		{

                        switch(writeWord(firstNumericParametre, secondNumericParametre))
                        {
                                case SECOND_PAGE_TABLE_ALLOCATION_ERROR: printf("Error allocating second page table\n"); return RET_FAILURE;
                                case PAGE_ALLOCATION_ERROR: printf("Error allocating page\n"); return RET_FAILURE;
                        }
		}
		return RET_SUCCESS;
	}
	
	
	// [s]tep
	if(!(strcmp(command, "step") && strcmp(command, "s")))
	{
		char* cpuInstruction, *disassembledInstruction;
		unsigned long regPC;

		regPC = getRegister("pc");
		cpuInstruction = readWordAsString(regPC);
		disassembledInstruction = decodeInstruction(cpuInstruction, regPC);
		printf("\t%08lX:\t", regPC);
		displayWord(cpuInstruction, 1);
		printf("\t%s\n",disassembledInstruction);
		executeInstruction(disassembledInstruction);

		free(cpuInstruction);
		free(disassembledInstruction);
		return RET_SUCCESS;
	}
	
	
        // [t]ra
	if(!(strcmp(command, "tra") && strcmp(command, "t")))
	{
		char equivalentSimulatorCommand[50];

                if(!firstParametre)
                    return RET_FAILURE;
                
                isVerbose = 1;
                strcpy(equivalentSimulatorCommand, "cont ");
                strcat(equivalentSimulatorCommand, firstParametre);
                processSimulatorCommand(equivalentSimulatorCommand);
                isVerbose = 0;
                
		return RET_SUCCESS;
	}
        
        
	// [c]ont
	if(!(strcmp(command, "cont") && strcmp(command, "c")))
	{
		int exitCode, instructionCount;

		instructionCount = 0;
                exitCode = RET_SUCCESS;
                
		if(!firstParametre)
		{
                    do
                    {
                        exitCode = executeNextInstruction();
                        switch(exitCode)
                        {
                            case RET_BREAKPOINT: 
                                printf("Breakpoint(%d) encountered at: 0x%08lX after executing %d instructions\n", getBreakPointSerial(), getRegister("pc"), instructionCount);
                                return RET_SUCCESS;
                            case RET_WATCHPOINT:
                                watchInfo = getWatchPointInfo();
                                printf("Watchpoint(%d) encountered at: 0x%08lX after executing %d instructions, Data address: 0x%08lX, New data: 0x%08lX\n", getWatchPointSerial(), getRegister("pc"), instructionCount, watchInfo->memoryAddress, watchInfo->newData);
                                free(watchInfo);
                                return RET_SUCCESS;
                            case RET_SUCCESS:
                                instructionCount++;
                                if(isVerbose)
                                {
                                    printf("\t%08lX:\t", lastInstructionInfo.regPC);
                                    displayWord(lastInstructionInfo.cpuInstruction, 1);
                                    printf("\t%s\n",lastInstructionInfo.disassembledInstruction);
                                }
                                break;
                            case RET_FAILURE: 
                                return RET_FAILURE;
                        }
                    }
                    while(exitCode == RET_SUCCESS);
		}
                
		else
		{
                    exitCode = RET_SUCCESS;
                    for(instructionCount = 0; instructionCount < firstNumericParametre; instructionCount++)
                    {
                        exitCode = executeNextInstruction();
                        switch(exitCode)
                        {
                            case RET_BREAKPOINT: 
                                printf("Breakpoint(%d) encountered at: 0x%08lX after executing %d instructions\n", getBreakPointSerial(), getRegister("pc"), instructionCount);
                                return RET_SUCCESS;
                            case RET_WATCHPOINT:
                                watchInfo = getWatchPointInfo();
                                printf("Watchpoint(%d) encountered at: 0x%08lX after executing %d instructions, Data address: 0x%08lX, New data: 0x%08lX\n", getWatchPointSerial(), getRegister("pc"), instructionCount, watchInfo->memoryAddress, watchInfo->newData);
                                free(watchInfo);
                                return RET_SUCCESS;
                            case RET_SUCCESS:
                                if(isVerbose)
                                {
                                    printf("\t%08lX:\t", lastInstructionInfo.regPC);
                                    displayWord(lastInstructionInfo.cpuInstruction, 1);
                                    printf("\t%s\n",lastInstructionInfo.disassembledInstruction);
                                }
                                break;
                        }
                    }
		}

		return RET_SUCCESS;
	}


	// [g]o
	if(!(strcmp(command, "go") && strcmp(command, "g")))
	{
                char equivalentSimulatorCommand[50];
                
                if(!firstParametre)
			return RET_FAILURE;
                
                // Initializing execution environment
		setRegister("pc", firstNumericParametre);
                setRegister("npc", firstNumericParametre + 4);
                setRegister("i6", 0x40400000);
                setRegister("o6", 0x403FFE80);
                setRegister("wim", 0x0000002);
                setRegister("psr", 0xF30010E0);

                strcpy(equivalentSimulatorCommand, "cont ");
                
                if(secondParametre)
                        strcat(equivalentSimulatorCommand, secondParametre);

                processSimulatorCommand(equivalentSimulatorCommand);
                
		return RET_SUCCESS;
	}


	// [ru]n
	if(!(strcmp(command, "run") && strcmp(command, "ru")))
	{
		char equivalentSimulatorCommand[50];

		setRegister("pc", 0);
                setRegister("npc", 4);
                
                strcpy(equivalentSimulatorCommand, "cont ");
                
		if(firstParametre)
                        strcat(equivalentSimulatorCommand, firstParametre);

                processSimulatorCommand(equivalentSimulatorCommand);
		return RET_SUCCESS;
	}


	// [br]eak
	if(!(strcmp(command, "break") && strcmp(command, "br")))
	{
		if(firstParametre == NULL)
		{
			struct breakPoint* curBreakPoint;
			
			curBreakPoint = getBreakPoint(1);
			if(!curBreakPoint)
				return RET_SUCCESS;

			do
			{
                                char* breakPointType = (curBreakPoint->breakPointType == BREAK_POINT) ? "Breakpoint" : "Watchpoint";
                                printf("\n%d: 0x%08lX -- %s", curBreakPoint->breakPointSerial, curBreakPoint->memoryAddress, breakPointType);
				curBreakPoint = getBreakPoint(0);
			}while(curBreakPoint);
			printf("\n\n");
		}
		
		else
                if(addBreakPoint(wordAlign(firstNumericParametre), BREAK_POINT) == BREAKPOINT_ALLOCATION_ERROR)
                {
                        printf("ERROR: Can't allocate breakpoint\n");
                        return RET_FAILURE;
                }
			
		return RET_SUCCESS;
	}
	
        
        // [wa]tch
	if(!(strcmp(command, "watch") && strcmp(command, "wa")))
	{
		
                if(addBreakPoint(wordAlign(firstNumericParametre), WATCH_POINT) == BREAKPOINT_ALLOCATION_ERROR)
                {
                        printf("ERROR: Can't allocate watchpoint\n");
                        return RET_FAILURE;
                }

                return RET_SUCCESS;
	}
	
        
	// [de]l
	if(!(strcmp(command, "del") && strcmp(command, "de")))
	{
		if(firstParametre == NULL)
			return RET_FAILURE;
		else
		{
			if(deleteBreakPoint(firstNumericParametre)  == RET_FAILURE)
				printf("ERROR: Breakpoint does not exist\n");
			else			
				return RET_SUCCESS;
		}
	}


	// [e]cho
	if(!(strcmp(command, "echo") && strcmp(command, "e")))
	{
		if(firstParametre == NULL)
			return RET_FAILURE;
		else
		{
			puts(firstParametre);
			return RET_SUCCESS;
		}
	}
	
	
	// [sh]ell
	if(!(strcmp(command, "shell") && strcmp(command, "sh")))
	{
		if(firstParametre == NULL)
			return RET_FAILURE;
		else
		{
			char* argumentBase = arguments;
			while(*arguments++ != ' ' );
			printf("\n");
			system(arguments);
			printf("\n");
			free(argumentBase);
			return RET_SUCCESS;
		}
	}
	
	
        //[r]eg
        if(!(strcmp(command, "reg") && strcmp(command, "r")))
        {
                char* registerValue, *cpuInstruction, *disassembledInstruction;
                char sparcRegister[3];
                unsigned long regPC;
                unsigned short count, window, currentRegisterWindow;
                sparcRegister[2] = '\0';

                // Record present register window
                currentRegisterWindow = getRegisterWindow();
                
                // Display different register window
                if(firstParametre != NULL && secondParametre == NULL)
                {
                        window = firstParametre[1] - '0';
                        setRegisterWindow(window);
                }
                
                // Set a register value
                if(firstParametre != NULL && secondParametre != NULL)
                        setRegister(firstParametre, secondNumericParametre);
                
                // Display processor registers
                else
                {
                        printf("\n\t\t    INS\t\t   LOCALS\t    OUTS\t  GLOBALS\n\n");

                        for(count = 0; count < 8; count++)
                        {
                                sparcRegister[1] = count + '0'; 

                                sparcRegister[0] = 'i'; 
                                registerValue = displayRegister(getRegister(sparcRegister));
                                printf("\t%d:\t %s", count, registerValue);
                                free(registerValue);


                                sparcRegister[0] = 'l'; 
                                registerValue = displayRegister(getRegister(sparcRegister));
                                printf("\t %s", registerValue);
                                free(registerValue);

                                sparcRegister[0] = 'o'; 
                                registerValue = displayRegister(getRegister(sparcRegister));
                                printf("\t %s", registerValue);
                                free(registerValue);

                                sparcRegister[0] = 'g';
                                registerValue = displayRegister(getRegister(sparcRegister));
                                printf("\t %s\n", registerValue);
                                free(registerValue);
                        }

                        registerValue = displayRegister(getRegister("psr"));
                        printf("\n\tpsr: %s", registerValue);
                        free(registerValue);

                        registerValue = displayRegister(getRegister("wim"));
                        printf("\t\twim: %s", registerValue);
                        free(registerValue);

                        registerValue = displayRegister(getRegister("tbr"));
                        printf("\t\ttbr: %s", registerValue);
                        free(registerValue);

                        registerValue = displayRegister(getRegister("y"));
                        printf("\t\ty: %s\n\n", registerValue);
                        free(registerValue);

                        regPC = getRegister("pc");
                        registerValue = displayRegister(regPC);
                        cpuInstruction = readWordAsString(regPC);
                        disassembledInstruction = decodeInstruction(cpuInstruction, regPC);
                        printf("\tpc : %s\t\t", registerValue);
                        displayWord(cpuInstruction, 1);
                        printf("\t%s", disassembledInstruction);
                        free(cpuInstruction);
                        free(disassembledInstruction);
                        free(registerValue);

                        regPC = getRegister("npc");
                        registerValue = displayRegister(regPC);
                        cpuInstruction = readWordAsString(regPC);
                        disassembledInstruction = decodeInstruction(cpuInstruction, regPC);
                        printf("\n\tnpc: %s\t\t", registerValue);
                        displayWord(cpuInstruction, 1);
                        printf("\t%s", disassembledInstruction);
                        
                        free(cpuInstruction);
                        free(disassembledInstruction);
                        free(registerValue);

                        if(getIUErrorMode())
                            color("\n\n\tIU in Error mode", 0);
                        
                        printf("\n\n");
                        
                        // Set register window back to present one
                        setRegisterWindow(currentRegisterWindow);
                }
                
                return RET_SUCCESS;
        }


        // [p]sr
        if(!(strcmp(command, "psr") && strcmp(command, "p")))
        {
            unsigned long regPSR;
            struct processor_status_register psr;

            regPSR = getRegister("psr");
            psr = castUnsignedLongToPSR(regPSR);

            printf("\n\tPSR = 0x%08lX\n\n", regPSR);
            printf("\tCWP = %d\n", psr.cwp);
            printf("\tET  = %d", psr.et); (psr.et) ? printf(" (Trap is enabled)\n") : printf(" (Trap is disabled)\n");
            printf("\tPS  = %d\n", psr.ps);
            printf("\tS   = %d", psr.s); (psr.s) ? printf(" (Supervisor mode)\n") : printf(" (User mode)\n");
            printf("\tPIL = %d\n", psr.pil);
            printf("\tEF  = %d", psr.ef); (psr.ef) ? printf(" (FPU is enabled)\n") : printf(" (FPU is disabled)\n");
            printf("\tEC  = %d\n", psr.ec);
            printf("\tRES = %d\n", psr.reserved);
            printf("\tC   = %d", psr.c); (psr.c) ? printf(" (Carry)\n") : printf("\n");
            printf("\tV   = %d", psr.v); (psr.v) ? printf(" (Overflow)\n") : printf("\n");
            printf("\tZ   = %d", psr.z); (psr.z) ? printf(" (Zero)\n") : printf("\n");
            printf("\tN   = %d", psr.n); (psr.n) ? printf(" (Negative)\n") : printf("\n");
            printf("\tVER = %d\n", psr.ver);
            printf("\tIMP = %d\n\n", psr.impl);

            return RET_SUCCESS;
        }

        
        // [fs]r
        if(!(strcmp(command, "fsr") && strcmp(command, "fs")))
        {
            unsigned long regFSR;
            struct floating_point_state_register fsr;

            regFSR = getRegister("fsr");
            fsr = castUnsignedLongToFSR(regFSR);

            printf("\n\tFSR   = 0x%08lX\n\n", regFSR);
            printf("\tcexc  = %d", fsr.cexc);
            if(fsr.cexc)
            {
                printf(" (");
                if(fsr.cexc & 0b10000)
                    printf("nvc, ");
                if(fsr.cexc & 0b01000)
                    printf("ofc, ");
                if(fsr.cexc & 0b00100)
                    printf("ufc, ");
                if(fsr.cexc & 0b00010)
                    printf("dzc, ");
                if(fsr.cexc & 0b00001)
                    printf("nxc, ");
                printf("\b\b \b \b)");
            }
            printf("\n\taexc  = %d", fsr.aexc);
            if(fsr.aexc)
            {
                printf(" (");
                if(fsr.aexc & 0b10000)
                    printf("nva, ");
                if(fsr.aexc & 0b01000)
                    printf("ofa, ");
                if(fsr.aexc & 0b00100)
                    printf("ufa, ");
                if(fsr.aexc & 0b00010)
                    printf("dza, ");
                if(fsr.aexc & 0b00001)
                    printf("nxa, ");
                printf("\b\b \b \b)");
            }
            printf("\n\tfcc   = %d (", fsr.fcc);
            switch (fsr.fcc)
            {
                case 0: printf("Equal)"); break;
                case 1: printf("Less than)"); break;
                case 2: printf("Greater than)"); break;
                case 3: printf("Unordered)"); break;
            }
            printf("\n\tulow  = %d\n", fsr.ulow);
            printf("\tqne   = %d (", fsr.qne);
            if(fsr.qne)
                printf("FQ non-empty)");
            else
                printf("FQ empty)");
            printf("\n\tftt   = %d (", fsr.ftt);
            switch (fsr.ftt)
            {
                case 0: printf("None)"); break;
                case 1: printf("IEEE_754_exception)"); break;
                case 2: printf("unfinished_FPop)"); break;
                case 3: printf("unimplemented_FPop)"); break;
                case 4: printf("sequence_error)"); break;
                case 5: printf("hardware_error)"); break;
                case 6: printf("invalid_fp_register)"); break;
                case 7: printf("reserved)"); break;
            }
            printf("\n\tver   = %d", fsr.ver);
            if(fsr.ver == 7)
                printf(" (No floating point controller present)");
            printf("\n\tres   = %d\n", fsr.res);
            printf("\tNS    = %d\n", fsr.ns);
            printf("\tTEM   = %d", fsr.tem);
            if(fsr.tem)
            {
                printf(" (");
                if(fsr.tem & 0b10000)
                    printf("NVM, ");
                if(fsr.tem & 0b01000)
                    printf("OFM, ");
                if(fsr.tem & 0b00100)
                    printf("UFM, ");
                if(fsr.tem & 0b00010)
                    printf("DZM, ");
                if(fsr.tem & 0b00001)
                    printf("NXM, ");
                printf("\b\b \b \b)");
            }
            printf("\n\tuhigh = %d\n", fsr.uhigh);
            printf("\tRD    = %d (", fsr.rd);
            switch(fsr.rd)
            {
                case 0: printf("Nearest (even, if tie))\n"); break;
                case 1: printf("Towards zero)\n"); break;
                case 2: printf("Towards positive infinity)\n"); break;
                case 3: printf("Towards negative infinity)\n"); break;
            }
            printf("\n");

            return RET_SUCCESS;
        }
        
        
        // [tb]r
        if(!(strcmp(command, "tbr") && strcmp(command, "tb")))
        {
            unsigned long regTBR, TBA;
            unsigned short zero, tt;

            regTBR = getRegister("tbr");
            zero = regTBR & 0x0000000F;
            tt = (regTBR & 0x00000FF0) >> 4;
            TBA = (regTBR & 0xFFFFF000) >> 12;
            
            printf("\n\tTBR  = 0x%08lX\n\n", regTBR);
            printf("\tzero = %d\n", zero);
            printf("\ttt   = 0x%X", tt);
            if(tt)
            {
                printf(" (");
                switch(tt)
                {
                    case ILLEGAL_INSTRUCTION: printf("illegal_instruction)"); break;
                    case PRIVILEGED_INSTRUCTION: printf("privileged_instruction)"); break;
                    case FP_DISABLED: printf("fp_disabled)"); break;
                    case WINDOW_OVERFLOW: printf("window_overflow)"); break;
                    case WINDOW_UNDERFLOW: printf("window_underflow)"); break;
                    case MEM_ADDRESS_NOT_ALIGNED: printf("mem_address_not_aligned)"); break;
                    case FP_EXCEPTION: printf("fp_exception)"); break;
                    case TAG_OVERFLOW: printf("tag_overflow)"); break;
                    case UNIMPLEMENTED_FLUSH: printf("unimplemented_flush)"); break;
                    case DIVISION_BY_ZERO: printf("division_by_zero)"); break;
                    case DESIGN_UNIMP: printf("design_unimplemented)"); break;
                }
            }
            printf("\n\tTBA  = 0x%lX\n\n", TBA);
            
            return RET_SUCCESS;
        }
        
        
        // [f]loat
        if(!(strcmp(command, "float") && strcmp(command, "f")))
	{
		char* registerValue;
		char sparcRegister[4];
		unsigned short count;
                sparcRegister[0] = 'f';
		sparcRegister[3] = '\0';

		registerValue = displayRegister(getRegister("fsr"));
                printf("\n\tfsr: \t%s\n\n", registerValue);
                free(registerValue);
			
                for(count = 0; count < 32; count++)
                {
                        sparcRegister[1] = (count / 10) + '0';
                        sparcRegister[2] = (count % 10) + '0';
                        
                        // Hex to single precision floating-point conversion
                        convertFloat.floatToHex = getRegister(sparcRegister);
                        
                        // Hex to double precision floating-point conversion
                        if(count % 2 == 0)
                        {
                            convertDouble.doubleToHex[0] = convertFloat.floatToHex;
                            convertDouble.doubleToHex[1] = getRegister(getNextRegister(sparcRegister));
                        }
                        else
                            convertDouble.hexToDouble = 0;
                            
                        registerValue = displayRegister(convertFloat.floatToHex);
                        if(count % 2 == 0)
                            printf("\tf%02d:\t%s\t%13E\t%13lE\n", count, registerValue, convertFloat.hexToFloat, convertDouble.hexToDouble);
                        else
                            printf("\tf%02d:\t%s\t%13E\n", count, registerValue, convertFloat.hexToFloat);
                        free(registerValue);
                }
                printf("\n");
                
		return RET_SUCCESS;
	}
        
        
	// [d]is
	if(!(strcmp(command, "dis") && strcmp(command, "d")))
	{
		if(firstParametre == NULL && secondParametre == NULL)
			return RET_FAILURE;
		else
			if(secondParametre == NULL)
				secondNumericParametre = 16;
			
		unsigned long instructionCount;
		char* cpuInstruction = NULL;
		char* disassembledInstruction = NULL;
		
		for(instructionCount = 0; instructionCount < secondNumericParametre; instructionCount++)
		{
			cpuInstruction = readWordAsString(firstNumericParametre);
			disassembledInstruction = (char*)decodeInstruction(cpuInstruction, firstNumericParametre);
			printf("\n\t");
			sprintf(hexNumber, "%lx", firstNumericParametre);
			printf("0x%s:\t", hexNumber);
			displayWord(cpuInstruction, 1);
			printf("\t%s", disassembledInstruction);
			firstNumericParametre += 4;
		}
		
		free(cpuInstruction);
		free(disassembledInstruction);
		printf("\n\n");
		return RET_SUCCESS;
	}
	
	else
		return RET_FAILURE;
}
