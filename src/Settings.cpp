/** @file Settings.cpp

  Copyright (C) 2008 Peer Sommerlund

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as 
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
*/
#include "Settings.hpp"

#include "config.h"
//#define DEB1
#include "Trace.hpp"
#include <iso646.h>

using std::string;
using std::vector;
using std::map;
using std::endl;
using std::istringstream;
using std::ostringstream;

void InitFile::Section::GetConfiguration(Settings& settings) const
{
  for (size_t j=0; j<lines.size(); j++) {
    // Extract those lines not beginning with '#' or ';', containing a '='
    const string& line = lines[j];
    if (line[0]=='#') continue;
    if (line[0]==';') continue;
    size_t sep = line.find('=');
    if (sep!=string::npos) {
      settings[string(line,0,sep)] = string(line,sep+1);
    }
  }
}

void InitFile::Section::SetConfiguration(const Settings& _settings)
{
  Settings remaining = _settings;

  //
  // Update existing lines
  //
  
  for (size_t j=0; j<lines.size(); j++) {
    // Skip lines beginning with '#' or ';', or not containing a '='
    string& line = lines[j];
    if (line[0]=='#') continue;
    if (line[0]==';') continue;
    size_t sep = line.find('=');
    if (sep!=string::npos) {
      // Update the value part of the line, but only if
      // settings contains the corresponding key
      string key = string(line,0,sep);
      Settings::iterator newSetting = remaining.find(key);
      if (newSetting != remaining.end()) {
        line.replace(sep+1, string::npos, newSetting->second);
        remaining.erase(newSetting);
      }
    }
  }

  //
  // Add new lines
  //

  // Add blank line, if lines is empty
  if (lines.size()==0) lines.push_back("");
  // Find last line that is non-blank
  vector<string>::reverse_iterator lastNonBlank;
  for (lastNonBlank = lines.rbegin(); lastNonBlank != lines.rend(); lastNonBlank ++) {
    if (*lastNonBlank != "") break;
  }
  // Insert after last non-blank line
  // The base() of reverse iterators are the forward iterator after that element
  vector<string>::iterator insertHere = lastNonBlank.base();
  int insertHereIndex = insertHere - lines.begin();
  string emptyLine;
  lines.insert(insertHere,remaining.size(),emptyLine); // make room
  // insertHere is invalid now
  for (Settings::iterator i = remaining.begin();
    i != remaining.end();
    i++)
  {
    string line = i->first + "=" + i->second;
    lines[insertHereIndex++] = line;
  }
}

/** Will append all sections read to the sections variable.
It is not a good idea to use Read twice on the same class because
duplicate named sections will shadow each other.

Lines starting with '#' or ';' or blank lines are comment lines.
A non-comment line that does not contain '=' is converted to
a comment line to make it clear that it has been igored.

@todo Add check for duplicate named sections
*/
void InitFile::Read(std::istream& in)
{
  Section curSec;
  std::string line;
  while (getline(in,line)) {
    if (line[0]=='[') {
      // Save current section and begin a new one
      sections.push_back(curSec);
      Section empty;
      curSec = empty;
      // Remove trailing space
      while (line[line.length()-1]==' ') line.erase(line.length()-1,1);
      // Remove leading and trailing brackets '[' ']'
      curSec.name = string(line,1,line.length()-2);
    }
    else if (line.find('=')==string::npos) {
      // This will be a comment line.
      // If the line contains text but does not start with a comment marker,
      // insert '#' to make it clear that this line is ignored.
      if (line.length()>0 and line[0]!='#' and line[0]!=';')
        line.insert(0,"# ");
      curSec.lines.push_back(line);
    }
    else {
      // This is an assignment line containing a '='.
      curSec.lines.push_back(line);
    }
  }
  // Save the last section read
  sections.push_back(curSec);
}

void InitFile::Write(std::ostream& out) const
{
  for (unsigned i=0; i<sections.size(); i++) {
    // If section 0 is named "" then it contains lines before first section
    // so don't write section name
    if (not (i==0 and sections[i].name==""))
      out << '[' << sections[i].name << ']' << endl;
    for (unsigned j=0; j<sections[i].lines.size(); j++) {
      out << sections[i].lines[j] << endl;
    }
  }
}

bool InitFile::GetSettings(std::string name, Settings& settings) const
{
  for (unsigned i=0; i<sections.size(); i++) {
    if (sections[i].name==name) {
      sections[i].GetConfiguration(settings);
      return true;
    }
  }
  Settings empty;
  settings = empty;
  return false;
}

void InitFile::SetSettings(std::string name, const Settings& settings)
{
  for (unsigned i=0; i<sections.size(); i++) {
    if (sections[i].name==name) {
      // Update existing section
      sections[i].SetConfiguration(settings);
      return;
    }
  }
  // Add a new section
  Section newSection;
  newSection.name = name;
  newSection.SetConfiguration(settings);
  sections.push_back(newSection);
}

std::vector<std::string> InitFile::GetSectionNames() const
{
  std::vector<std::string> result;
  for (unsigned i=0; i<sections.size(); i++) {
    result.push_back(sections[i].name);
  }
  return result;
}

