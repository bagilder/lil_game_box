/* choose your own parser
bgilder 2022 mar 16
ingest a marked up text file to build a choose your own adventure state machine */

/* 
rev 0 initial commit
*/

/*
state machine within an array of structs. each struct is its own state, all the structs are stored in an array, 
and the next state can just be navigated as the array index chosen by the option int selected in the currently active state
{
	text: array of narrative strings
	choice text: array of option strings
	options: array of ints (next-state destinations)
}
*/

/*
<1
this is the beginning of "state" 1.
each line can only be 40 char max.
>this is the beginning of a new screen
within the same "state"
~this is a choice.^4 (that points to state 4)
~this is a choice^5
~this is a choice.
choices can be multiple lines. but
mind the character count ^7

<5
here is the beginning of "state" 5.
each screen holds 312 characters max.
markup doesn't count against chars
etc

<# beginning of new state and index (# is not restricted)
> beginning of new screen in same state
~ branch option
^# branch destination and index
\ restricted for headache prevention 

can keep indeces literal. could use [0] for title info e.g. altho perhaps a dedicated system for titles would be better.	//i like the idea of dedicated title handling
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_CHAR_COUNT 41	//extra 1 for null terminator
#define MAX_ROW_COUNT 8
#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
//characters have leading blank pixel and blank lower pixel (top right justified)
#define MAX_CONTINUES 10
#define MAX_CHOICES 8
#define MAX_READ_LENGTH 50

#define BUF_SIZE 65536 // for state read fxn. can surely be shortened


U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 9, /* dc=*/ 7, /* reset=*/ 8);	// Enable U8G2_16BIT in u8g2.h



char filename = "story.txt";	//the input narrative script file
char fileout = "story.h";	//the output structure file

long maxNodes = 1; 	//the number of states in our state machine. will be updated on initial full file read


//this section must go after we open both files.




/*	//i'm choosing not to do this because we don't really need nodes in memory. just write that shit to file
int initNode(Data *datastruct, char *narrstr, char *chstr, int selnum)
{
	if (datastruct == NULL)
    {	return -1;
    }

	while(//we haven't hit the choices yet)
	{
		while(//we haven't hit a null character yet )
		{
    		memmove(&datastruct->narrativeString[i], fn, strlen(narrativeLineCount)+1);	//this is not the way to do this. 
    	}
		while(//we haven't hit a null character or ^ yet )
    	{
    		memmove(&datastruct->choiceString[i], fn, strlen(choiceLineCount)+1);	//this is not the way to do this.
    		    	}
	}

	for(int narrativePad = narrativeLineCount; narrativePad < MAX_CHAR_COUNT; ++narrativePad)	//this might have an off-by-one error //do i need an if(linecount < max_count) check first? i wouldn't expect so
	{
		datastruct->narrativeString[narrativePad] = " ";	//hopefully the leading ++ doesn't overwrite the last character of the actual array
	}

	for(int choicePad = choiceLineCount; choicePad < MAX_CHAR_COUNT; ++choicePad)	//this might have an off-by-one error
	{
		datastruct->choiceString[choicePad] = " ";	//hopefully the leading ++ doesn't overwrite the last character of the actual array
	}

	return 0;
}
*/

int read_file()		//hey idiot don't forget to escape \ and " in the text
{
//ingest text file

    char c; //to copy characters in. this is super inefficient
    
    FILE *instream = fopen(filename, "r");	//read only
    FILE *outstream = fopen(fileout, "w"); // write only
    if (instream == NULL || outstream == NULL) {
        // perror prints the reason for the failure
        perror("could not open text file");
        // If the file doesn't open, you should exit.
        exit(EXIT_FAILURE);
    } 


    //while(!feof(p) && !ferror(p))

    int totalStateNum = count_states(filename);		//can i call a FILE while already having that FILE open? may just move that code up here instead of as a function 

    fseek(instream, 0, SEEK_SET);	//scroll to the beginning of the file. this might not be necessary if we aren't writing to that file but idk


    
	fprintf(outstream, "#include <pgmspace.h>\n\n");
	fprintf(outstream, "typedef struct Data{\n"
	fprintf(outstream, "\tchar narrativeString[%i][%i][%i];\n",MAX_CONTINUES,MAX_ROW_COUNT,MAX_CHAR_COUNT);
	fprintf(outstream, "\tchar choiceString[%i];\n", MAX_CHOICES);
	fprintf(outstream, "\tchar selectNum[%i];\n",MAX_CHOICES);
	fprintf(outstream, "} Data;\n\n");
	fprintf(outstream, "const Data theData[%i] PROGMEM = {\n", count_states(filename)+1);	//if this works, then hooo boy. if not, use totalStateNum+1 like a boring person


    char *buffer;
    size_t bufsize = MAX_READ_LENGTH;
    size_t num_copied; 

    buffer = (char *)malloc(bufsize * sizeof(char));	//assign memory to buffer
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");	//if we're somehow out of memory
        exit(1);
    }

    num_copied = getline(&buffer,&bufsize,instream);	//alright we snagged the line woohoo. should now locally be in buffer

    switch (buffer[0])
    {
    	case "\n":	//if it is empty space
    	break;

    	case "<":	//if it is a state enumeration
    		if(choiceCount < MAX_CHOICES)
    		{
    			for(int pad = choiceCount; pad < MAX_CHOICES; pad++)
    			{	fprintf(outstream, ", 0");		//this fills out the choice int array. set to zero for executable choice count check
    			}
    		}
    		fprintf(outstream, "},\n//new state\n{"); 	//will this align things properly?	new state means new bracket, right?
    		screenCount = -1;
    		choiceCount = -1;
    	break;

    	case ">":
    		screenCount++;
    		if(screenCount > MAX_CONTINUES)
           	{	screenCount = MAX_CONTINUES;
           	}
           	int i=1;
           	while(buffer[i] != "\n" || buffer[i] != "\0")
           	{	i++;
           	}
           	fprintf(outstream, "\n{");
           	for(int crawl = 1; crawl < i; crawl++)	//go thru each position character by character
           	{	fprintf(outstream, "%c", buffer[crawl]);
           	}
           	fprintf(outstream, "\0}, ");
        break;

        case "~":
        	choiceCount++;
        	if(choiceCount > MAX_CHOICES)
        	{	choiceCount = MAX_CHOICES;
        	}
        	int i=1;
           	while(buffer[i] != "^" || buffer[i] != "\0" || buffer[i] != "\n")		///this won't catch multi-line choices yet
           	{	i++;
           	}
           	for(int crawl = 1; crawl < i; crawl++)	//go thru each position character by character
           	{
           		fprintf(outstream, "\n{");
           		fprintf(outstream, "%c", buffer[crawl]);
           	}
           	fprintf(outstream, "\0}, ");
           	int temp_num = 0;
           	i++;	//this should get me past the ^, right? delete if off by 1
           	while(buffer[i] != "\n" || buffer[i] != "\0")
           	{
           		if(buffer[i+1] != "\n" && buffer[i+1] != "\0" && buffer[i+1] != " ")
           		{	temp_num *= 10;
           		}
           		temp_num = temp_num + atoi(buffer[i]);	//if uint8_t for the next state, no need for atoi
           		//write to the file ", %i" temp_num		//this should still work even for
           	}
        break;
        
        default:
        {	fprintf(outstream, ", {%s}", buffer); 	///be sure that we catch the multi-line choice option above or else we won't get the right transitions. either that or handle that here
        }
        break;
    }


    if(buffer != NULL)		//do we need to free this once we're done so we don't kill our memory? is this good for clearing the previous line from buffer?
    {	free(buffer);
    }


//** ** ** change the below structure to read in whole lines from the source file and then do a position-by-position scan of the string in a temporary buffer, and switch off that.
    //then do whatever cleanup is needed to transfer the result into the local "storage" string and put it where it needs to go. don't read in each character from
    //the source file one at a time. that makes things harder. ** ** **



    for(int state = 0; state < totalStateNum; ++state)
    {
    	int charPos = -1;
    	int endPos = 0;		// to copy stuff between first of line and ^
    	int screenCount = 0;
    	int choiceCount = -1;
    	for (c = getc(instream); c != EOF; c = getc(instream))	//is this gonna double up on the first read?
    	{
            charPos++;
            switch (c)
            {
            	case "<":
            		while(c = getc(instream) != "\n")	//does this need to be \0?
            		{
            			c = getc(instream);		//is this gonna double up? would the while loop check do enough for us?
            		}
            		break;

            	case "\n":// Increment count if this character is newline
           			charPos=0;
           			lineCount++;
           			if(lineCount >= MAX_ROW_COUNT)
           			{	lineCount = 0;
           			}
           			break;

           		case ">":
           			screenCount++;
           			if(screenCount >= MAX_CONTINUES)
           			{	screenCount = 0;
           			}
           			break;

           		case "~":
           			choiceCount++;
           			if(choiceCount >= MAX_CHOICES)
           			{	choiceCount = -1;
           			}
           			break;

           		case "^":
           			// figure out the number of characters between here and \n and then atoi() that amount of stuff into the option destination. choiceCount will be the same
           			endPos = charPos;
           			break;

           	}	//end switch on c character
           	
           	if (feof(filename))	//will this do what i want? with the else if on c?
            	break;
        }
    }


//can i nest strtok() to break up \n as different tokens in narrativeStrings but then if it encounters ~ break those into choices?


/////////////////////////////////////////////
Character pointers can also be passed as parameters of strcpy.

#define DEST_SIZE 40

int main()
{
	char src[] = "Look Here";
	char dest[DEST_SIZE] = "Unimaginable";


	char *p = dest + 5;

	strcpy(p, src);
	printf(dest);

	return 0;
}

Output:
UnimaLook Here
/////////////////////////////////////////////


} 
/*
std::vector<char> v;
if (FILE *fp = fopen("filename", "r"))
{
	char buf[1024];
	while (size_t len = fread(buf, 1, sizeof(buf), fp))
		v.insert(v.end(), buf, buf + len);
	fclose(fp);
}
*/


int count_states(FILE* file)
{
    char buf[BUF_SIZE];
    int counter = 0;
    for(;;)
    {
        size_t res = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '<')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
}


//navigate to last line of file, line-- until char[0] == "<", read # to initialize struct array with full number of states. (do i need malloc?)

//every < gives us a new struct index
//every > gives us a fresh screen in the text array
//every ~ gives us a fresh option in the choice text array
//every ^ provides the corresponding desination for that choice

//we know every line will be 40 characters long
//we know there will be 8? number of lines per screen
//if either do not fill their buffers, insert blank space (whatever the lcd considers blank space)

//maybe build a separate title screen file? could allow us to draw graphics
//maybe eventually build a separate music file?



///don't forget to close the file streamsssss!

