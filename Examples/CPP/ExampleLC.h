/*	ExampleLC.h
 *
 *		This file was automatically generated by OCLex, part of the OCTools
 *	suite available at:
 *
 *		https://github.com/w3woody/OCTools
 */

#ifndef ExampleLC_h
#define ExampleLC_h

#include <stdint.h>
#include <string>


    %%


%header


/*	OCFileInput
 *
 *		The input file stream must correspond to this interface to read
 *	the contents of a file or data object. The byte returned is from 0 to 255,
 *	and EOF is marked with -1.
 */

#ifndef OCFileInputProtocolC
#define OCFileInputProtocolC

class OCFileInput
{
	public:
		virtual int readByte() = 0;
		virtual int peekByte() = 0;
};

#endif

/*	ExampleLC
 *
 *		The generated lexical parser
 */

class ExampleLC
{
	public:
		ExampleLC(OCFileInput *file);
		~ExampleLC(void);

		/*
		 *	Current reader state
		 */

		int32_t line;
		int32_t column;
		std::string filename;
		std::string text;
		std::string abort;

#ifdef ExampleLC_ValueDefined
		union ExampleLCValue value;
#endif

		void setFile(std::string &file, int32_t line);
		void setLine(int32_t line);
		int32_t lex();						// Method to read next token
%global


	private:
		// Files
		OCFileInput *file;

		// Read position support
		int32_t curLine;
		int32_t curColumn;

		// Mark location support
		int32_t markLine;
		int32_t markColumn;

		// Mark buffer storage
		bool isMarked;						// yes if we have mark set
		unsigned char *markBuffer;			// mark buffer
		int32_t markSize;					// bytes stored in buffer
		int32_t markAlloc;					// capacity of buffer

		// Read cache
		unsigned char *readBuffer;			// read cache buffer
		int32_t readPos;					// Read position
		int32_t readSize;					// size of data in read buffer
		int32_t readAlloc;					// Capacity of read cache

		// Text read buffer
		unsigned char *textBuffer;			// text cache for reading buffer
		int32_t textMarkSize;
		int32_t textSize;
		int32_t textAlloc;

		// State flags
		uint64_t  states;

		// Internal Methods
		void mark(void);
		void reset(void);
		int input(void);
		bool atEOL(void);
		bool atSOL(void);
		uint16_t stateForClass(uint16_t charClass, uint16_t state);
		uint16_t conditionalAction(uint16_t state);
%local

};

#endif