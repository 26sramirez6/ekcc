/*
 * config.hpp
 *
 *  Created on: Apr 30, 2019
 *      Author: 26sra
 */

#ifndef EKCC_COMPILERCONFIG_HPP_
#define EKCC_COMPILERCONFIG_HPP_

#include <iostream>
#include <string>
#include <string.h>

using std::cout;
using std::endl;
using std::string;
using std::equal;

struct CompilerConfig {
	bool help_ = false;
	bool emitAST_ = false;
	bool jit_ = false;
	int jitArgStart_ = 0;
	bool optimize_ = false;
	bool emitLLVM_ = false;
	bool properConfig_ = false;
	string outputFile_;
	char * inputFile_;

	CompilerConfig(int argc, char ** argv) {
		bool inputFileSet = false;
		bool outputFileSet = false;
		for (int i=1; i<argc; ++i) {
			if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-?")) {
				this->help_ = true;
			} else if (!strcmp(argv[i], "-emit-ast")) {
				this->emitAST_ = true;
			} else if (!strcmp(argv[i], "-emit-llvm")) {
				this->emitLLVM_ = true;
			} else if (!strcmp(argv[i], "-jit")) {
				this->jit_ = true;
			} else if (!strcmp(argv[i], "-O")) {
				this->optimize_ = true;
			} else if (!strcmp(argv[i], "-o")) {
				this->outputFile_ = string(argv[i+1]);
				outputFileSet = true;
			} else if (CompilerConfig::EndsWith(argv[i], ".ek")) {
				this->inputFile_ = argv[i];
				inputFileSet = true;
			}
		}

		if (this->jit_) {
			// pick up the start index of the arguments intended for "some_file.ek"
			// to handle something like "./ekcc -jit some_file.ek 3 4 1 2"
			for (int i=1; i<argc; ++i) {
				if (argv[i][0] != '-' &&
					!CompilerConfig::EndsWith(argv[i], ".ek") ) {
					this->jitArgStart_ = i;
					break;
				}
			}
		}

		if (outputFileSet && this->jit_) {
			this->properConfig_ = false;
		} else if (outputFileSet) {
			this->properConfig_ = inputFileSet && outputFileSet;
		} else {
			this->properConfig_ = inputFileSet && this->jit_;
		}

	}

	static bool EndsWith(string const & str,
			string const & suffix)
	{
	    if (suffix.size() > str.size())
	    	return false;

	    return equal(suffix.rbegin(),
	    		suffix.rend(), str.rbegin());
	}

	static bool StartsWith(string const & str,
				string const & prefix)
	{
		if (prefix.size() > str.size())
			return false;

		return equal(prefix.begin(),
				prefix.end(), str.begin());
	}

	void 
	PrintUsage() {
		cout << "Usage: [-h|-?] [-v] [-O] " <<
			"[-emit-ast|-emit-llvm] [-jit | -o <output-file>] " <<
			"<input-file>" << endl;
	}
};


#endif /* EKCC_COMPILERCONFIG_HPP_ */
