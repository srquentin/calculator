/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...(done)
   if (_dofileStack.size() > 1024) //prevent oversize
        return false;
   _dofileStack.push(_dofile); //store previous _dofile
   _dofile = new ifstream(dof.c_str());
   
   if (_dofile->fail()) { //if fail to open
        delete _dofile;   //delete
        _dofile = NULL;
        if (!_dofileStack.empty()) {
            _dofile = _dofileStack.top();
            _dofileStack.pop();
        }
        return false;
   }
   return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
    assert(_dofile != 0);
    // TODO...(done)   
    delete _dofile;
    _dofile = NULL;
    if (!_dofileStack.empty()) {
        _dofile = _dofileStack.top();
        _dofileStack.pop();
    }
}

// Return false if registration fails
// regCmd("HIStory", 3, new HistoryCmd)
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str; //HIS
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp); //tory
   assert(e != 0);
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
    // TODO...(done)
    for (CmdMap::const_iterator search = _cmdMap.begin(); search != _cmdMap.end(); search++)
    {
        search->second->help();
    }    
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
	assert(_tempCmdStored == false);
	assert(!_history.empty());
	string str = _history.back();
    assert(str[0] != 0 && str[0] != ' ');
    // TODO...(done)
	
	string firstword;
	CmdExec* result = 0;
    size_t word_end_pos = 0;

	word_end_pos = myStrGetTok(str, firstword); // 
	if ((result = getCmd(firstword)) == 0)
		cout << "Illegal command!! (" << firstword << ")" << endl;
	else {
		if (word_end_pos == string::npos) 
			option = ""; //no arg2
		else {
			word_end_pos = str.find_first_not_of(' ', word_end_pos);		
	    	option = str.substr(word_end_pos, str.size());
		}
	}	
	return result;
}

// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. [Before] Null cmd
//    cmd> $
//    -----------
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    -----------
//    [Before] partially matched (multiple matches)
//    cmd> h$aaa                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$aaa                // and then re-print the partial command
//
// 3. [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $
//    -----------
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$hahah
//    [After Tab]
//    cmd> heLp $hahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//
// 4. [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. [Before] Already matched
//    cmd> help asd$fg
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$fg
//
// 6. [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location
//
void
CmdParser::listCmd(const string& str)
{
    // TODO...
    //str is the partial string before current cursor position
    //_readBuf[READ_BUF_SIZE];// save the current line input
    string totalInput = _readBuf;
	size_t begin = str.find_first_not_of(' ', 0);
	string firstword;
    string complete_cmd;
    //size_t matchNum = 0;
    vector<string> vMatchCmd;
	vector<string>::iterator it;
    size_t i = 0;
    CmdMap::const_iterator search;
	CmdExec *e;
    //const char *cstring;
    
    if (begin == string::npos) { //case 1 : null cmd print all cmd    
		cout << endl;
        for (search = _cmdMap.begin(); search != _cmdMap.end(); search++, i++)
        {
            cout << setw(12) << left << search->first + search->second->getOptCmd();
            if (i == 4) {
                cout << endl;
                i = 0;
            }
        }      
        //re-print the prompt and place the cursor back to original location
        reprintCmd();
		return;
    }
    //else
    //    tr_string = str.substr(begin, str.size());
	
    //vMatchCmd.clear();
	myStrGetTok(str, firstword);
	if (firstword.size() == str.size()) //no second word
	{
	    for (search = _cmdMap.begin(); search != _cmdMap.end(); search++)
	    {
			complete_cmd = search->first + search->second->getOptCmd();
			if (firstword.size() > complete_cmd.size())
				continue;
			if (myStrNCmp(complete_cmd, firstword, firstword.size()) == 0) //equal
	        {
	            //matchNum++;
	            vMatchCmd.push_back(complete_cmd);
	        }
	    }
		if (vMatchCmd.size() > 1) //partially matched (multiple matches)
	    {
	        cout << endl;
			// List all the parially matched commands
			for(i = 0, it = vMatchCmd.begin(); it != vMatchCmd.end(); it++)
	        {
	            cout << setw(12) << left << *it;
				if (i == 4) {
	                cout << endl;
	                i = 0;
	            }
	        }
			// and then re-print the partial command
			reprintCmd();
			return;

	    }
	    else if (vMatchCmd.size() == 1) {
	        
	        complete_cmd = vMatchCmd[0];
	        //Already matched
	        //if (myStrNCmp(complete_cmd, vMatchCmd[0], complete_cmd.size()) == 0)
							
			if (complete_cmd.size() == firstword.size())
			{
				cout << endl;
				e = getCmd(complete_cmd);
				e->usage(cout);
				reprintCmd();
	        }
	        else //partially matched (single match)
	        {
	            //Automatically complete on the same line
	            //cout << complete_cmd.substr(firstword.size(), complete_cmd.size());
	            for (size_t i = firstword.size(); i < complete_cmd.size(); i++)
	            {
                //cstring = complete_cmd.substr(firstword.size(), complete_cmd.size()).c_str();
                	insertChar(complete_cmd[i]);
	            }
				insertChar(' ');
	        }
			return;
	    }

	    else //No match
	    {
			mybeep();
		}
	}
	else
	{
	    if ((e = getCmd(firstword))!= NULL)
		{
			cout << endl;
			e->usage(cout);
			reprintCmd();
		}
		else
		{
			mybeep();
		}
		
	}
        
    
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   // TODO...(done)
    CmdExec* e = 0;
    string optStr;
	string completeCmd;
	//size_t cmdsize = 0;
	for (CmdMap::iterator search = _cmdMap.begin(); search != _cmdMap.end(); search++)
	{
		//cmdsize = search->first.size();
		completeCmd = search->first + search->second->getOptCmd();
		//if ( myStrNCmpEx(search->first, cmd, cmdsize) == 0) //mandatory part
		if ( myStrNCmp(completeCmd, cmd, search->first.size()) == 0) //mandatory part
		{
			if (cmd.size() > search->first.size())
			{
				optStr = cmd.substr(search->first.size());
				if ( myStrNCmp(search->second->getOptCmd(),optStr, optStr.size()) == 0)
					e = search->second;
			}
			else
				e = search->second;
			break;
    	}
	}    
    return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

