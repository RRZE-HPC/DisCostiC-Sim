///    \file
///    CONFIGPARSER_HPP
///
///    \Author: Ayesha Afzal <ayesha.afzal@fau.de>
///    \Copyright © 2024 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#pragma once

#include <sstream>
#include <fstream>
#include <map>
#include <typeinfo>
#include <mpi.h>
#include "macro.hpp"


/**
 *\brief it parses the user-defined configuration file (.cfg)
*/
namespace UserInterface {
    /**
     *\brief a wrapper class which contain function for the conversion of std::string to primitive types (int, float, double, etc.,)
    */
    class Conversion
    {    
	public:
            template <typename T>
            static T stringTOScalarT(std::string const &val)
            {
                std::istringstream is(val);
                T Val;
                if (!(is >> Val)) throw std::runtime_error("CFG: invalid " + (std::string)typeid(T).name() + "\n");
                return Val;
            }
 
            template <typename T>
            static std::vector<T> stringTOArray(std::string const &val)
            {
                std::istringstream iss(val);
                std::vector<T> array;
                char c;
                T Val;
                if (iss >> c && c == '[')
                while (iss >> Val)
                    array.push_back(Val);
                if (iss && iss >> c && c == ']' && !(iss >> c))               
                return array;
            }           
    };
              
    /**
     *\brief a wrapper class which contains functions for parsing the configuration file
    */
    class ConfigParser
    {        
    private:
            std::map<std::string, std::string> data; //!< which will hold pairs of key-value
            std::string fileName; //!< As member variables, we will only have a std::string, which will hold the name of the configuration file
            
            /**
             \brief a function that removes everything from the semicolon (including it) to the end of the line.
             */
            void removeComment(std::string &line) const
            {
                if (line.find('#') != line.npos) line.erase(line.find('#'));
            }
            
            /**
             \brief a function that returns false if a non-space character was found, true otherwise. The function is "const" because it does not alter any class member variables.
             */
            bool whitespace(const std::string &line) const
            {
                return (line.find_first_not_of(' ') == line.npos);
            }
            
            /**
             *\brief a function that extracts the key from the pair of key = value
             */
            void extractKey(std::string &key, size_t const &sepPos, const std::string &line) const
            {
                key = line.substr(0, sepPos);
                if (key.find('\t') != line.npos || key.find(' ') != line.npos)
                    key.erase(key.find_first_of("\t "));
            }
            /**
             *\brief a function that extracts the value from the pair of key = value
             */
            void extractValue(std::string &value, size_t const &sepPos, const std::string &line) const
            {
                value = line.substr(sepPos + 1); // creates a substring starting from positon of '=' + 1, to the end of the line
                value.erase(0, value.find_first_not_of("\t ")); // removes the leading whitespace
                value.erase(value.find_last_not_of("\t ") + 1); // removes everything starting with the position of the last non-space character
            }
            
            /**
             *\brief a function that parse the line by calling above mentioed functions
             */
            void parseLine(const std::string &line, size_t const lineNum)
            {                                
	      		std::string temp = line;
                temp.erase(0, temp.find_first_not_of("\t "));
                size_t sepPos = temp.find('=');
                std::string key, value;
                extractKey(key, sepPos, temp);
                extractValue(value, sepPos, temp);
                
                if (!getKey(key)) // it checks if a key given as parameter already exists in the std::map (data)
                    data.insert(std::pair<std::string, std::string>(key, value));
                else
                    throw std::runtime_error("CFG: only unique key names are allowed!\n");
            }
        
	public:
            /**
             *\brief a class to set the name of the configuration file and extracts and parses the data
             */
	    	ConfigParser(const std::string &fileName)
            {
                
                std::ifstream file;				
                fileopen(file,fileName);
                
                std::string line;
                size_t lineNum = 0;
                while (std::getline(file, line))
                {
                    lineNum++;
                    std::string temp = line;
                    if (temp.empty()) continue;
                    
                    removeComment(temp);
            
                    if (whitespace(temp)) continue;
                    
                    parseLine(temp, lineNum);
                }
                
                fileclose(file,fileName);
                
            }
            /**
             *\brief a function for finding the key
             */
            bool getKey(const std::string &key) const
            {
                return data.find(key) != data.end();
            }
            /**
             *\brief a function that retrieves the value of a specific key
             */
            template <typename scalarT>
            scalarT getValue(const std::string &key, scalarT const &defaultValue = scalarT()) const
            {
                if (!getKey(key)) return defaultValue;
                return Conversion::stringTOScalarT<scalarT>(data.find(key)->second);
            }
            
    };	//!< end class configParser
} 		//!< end namespace useInterface
#endif 	//!< end CONFIGPARSER_HPP 
