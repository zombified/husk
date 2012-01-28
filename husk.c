// Joel Kleier
// CS 421

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


#define MAXWIDTH 256*32
#define MAXARGS 32
#define MAXARGSIZE 256 
char* cmd[MAXARGS];
char buf[MAXWIDTH];

#define NUMDOSCOMMANDS 9
char* dos[] = { "dir", "del", "copy", "move", "rename", "type", "md", "rd", "cls" };
char* nix[] = { "ls", "rm", "cp", "mv", "mv", "cat", "mkdir", "rmdir", "clear" };


void printcmd()
{
	int i;
	for( i = 0; i < MAXARGS; i++ )
		printf( "\n%d: %s", i, cmd[i] );
	fflush( stdout );
}

// I use a custom parse function because it allows arguments with escaped spaces and quoted arguments
void parse()
{
	unsigned brk[MAXWIDTH], i, s, d, q;
	q = 0;
	for( i = 0; i < strlen(buf); i++ )
	{
		if( q )
			if( buf[i] == '\"' )
			{
				q = 0;
				brk[i] = 0;
			}
			else
				brk[i] = 3;
		else
			switch( buf[i] )
			{
				case '\"' : brk[i] = 0; i++;
					break;
				case ' ' :
				case '\t' :
				case '\n' :
				case '\r' : brk[i] = 0;
					break;
				case '<' :
				case '|' : brk[i] = 1;
					break;
				case '\\' : brk[i] = 2; i++;
				default : brk[i] = 2;			
			}
	}

	d = 0;
	for( i = 0; i < strlen(buf); i++ )
	{
		if( brk[i] != 0 )
		{
			for( s = i; s < strlen(buf); s++ )
				if( brk[s] != brk[i] ) break;

			cmd[d] = (char*) calloc( MAXARGSIZE, sizeof(char) );
			strncpy( cmd[d], buf+i, s-i );
			d++;
			i = s-1;
		}
	}
}

void fixcmd()
{
	int i;

	// the "more" dos command gets special treatment. essentially we just delete the "<" argument
	if( strcmp( cmd[0], "more" ) == 0 && strcmp( cmd[1], "<" ) == 0 )
	{
		free( cmd[1] );
		cmd[1] = NULL;
		
		for( i = 1; i < MAXARGS-1; i++ )
			cmd[i] = cmd[i+1];
		
		cmd[MAXARGS-1] = NULL;
		
		return;
	}
	
	for( i = 0; i < NUMDOSCOMMANDS; i++ )
		if( strcmp( cmd[0], dos[i] ) == 0 )
		{
			free(cmd[0]);
			cmd[0] = nix[i];
			break;
		}
}


main()
{
	int i;
	pid_t pid;
	while(1)
	{
		for( i = 0; i < MAXARGS; i++ )
		{
			if( cmd[i] != NULL )
				free(cmd[i]);
			cmd[i] = NULL;
		}
			
		getcwd( buf, MAXWIDTH );
		printf( "\n%s$ ", buf );
		fgets( buf, MAXWIDTH, stdin );

		parse();
		fixcmd();

		pid = fork();
		if( pid == 0 )
		{
			execvp( cmd[0], cmd );
			printf( "\nCommand '%s' not found.\n", cmd[0] );
			exit(0);
		}

		wait(0);			
	}

	exit(0);
}
