///    \file
///    CONFIGPARSER_HPP
///
///    \Author: Ayesha Afzal <ayesha.afzal@fau.de>
///    \Copyright © 2024 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///

#ifndef NETWORKCONFIGPARSER_HPP
#define NETWORKCONFIGPARSER_HPP

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
namespace UserInterface
{

    /**
     *\brief a wrapper class which contains functions for parsing the configuration file
     */
    class NetworkConfigParser
    {
    private:
        int dataCounter = 0;
        double networkFileData[400][8] = {0};
        std::map<std::string, std::string> data; //!< which will hold pairs of key-value
        std::string fileName;                    //!< As member variables, we will only have a std::string, which will hold the name of the configuration file

        /**
         \brief a function that removes everything from the semicolon (including it) to the end of the line.
         */
        void removeComment(std::string &line) const
        {
            if (line.find('#') != line.npos)
                line.erase(line.find('#'));
        }

        /**
         \brief a function that returns false if a non-space character was found, true otherwise. The function is "const" because it does not alter any class member variables.
         */
        bool whitespace(const std::string &line) const
        {
            return (line.find_first_not_of(' ') == line.npos);
        }

        /**
         *\brief a function that parse the line by calling above mentioed functions
         */
        void parseLine(const std::string &line, size_t const lineNum)
        {
            std::string temp = line;
            // std::cout << line << std::endl;

            size_t pos = 0;
            std::string token;

            size_t index = 0;

            while ((pos = temp.find(",")) != std::string::npos)
            {
                token = temp.substr(0, pos);
                networkFileData[dataCounter][index] = stod(token);
                // std::cout << "Token : "<<token << std::endl;
                ++index;
                temp.erase(0, pos + std::string(",").length());
            }
            networkFileData[dataCounter][index] = stod(temp);

            ++dataCounter;
            // for (int i = 0; i < dataCounter; ++i)
            // {
            //     for (int j = 0; j < 8; ++j)
            //     {
            //         std::cout << networkFileData[i][j] << std::endl;
            //     }
            // }
        }

    public:
        /**
         *\brief a class to set the name of the configuration file and extracts and parses the data
         */

        NetworkConfigParser()
        {
        }
        NetworkConfigParser(const std::string &fileName)
        {

            std::ifstream file;
            fileopen(file, fileName);

            std::string line;
            size_t lineNum = 0;
            while (std::getline(file, line))
            {
                lineNum++;
                std::string temp = line;
                if (temp.empty())
                    continue;

                removeComment(temp);

                if (whitespace(temp))
                    continue;

                parseLine(temp, lineNum);
            }

            fileclose(file, fileName);
        }

        void readData(const std::string &fileName)
        {

            std::ifstream file;
            fileopen(file, fileName);

            std::string line;
            size_t lineNum = 0;
            while (std::getline(file, line))
            {
                lineNum++;
                std::string temp = line;
                if (temp.empty())
                    continue;

                removeComment(temp);

                if (whitespace(temp))
                    continue;

                parseLine(temp, lineNum);
            }

            fileclose(file, fileName);
        }

        void setData(const int buffersize)
        {
            int index = 0;
            for (int i = 0; i < dataCounter - 1; ++i)
            {
                if (networkFileData[i][0] < buffersize && buffersize < networkFileData[i + 1][0])
                {
                    if ((buffersize - networkFileData[i][0]) < (networkFileData[i + 1][0] - buffersize))
                    {
                        index = i;
                    }
                    else
                    {
                        index = i + 1;
                    }
                }
                else
                {
                    index = dataCounter - 1;
                }
            }
            data.insert(std::pair<std::string, std::string>("latency_in_ns", std::to_string(networkFileData[index][1])));
            data.insert(std::pair<std::string, std::string>("G_in_ns", std::to_string(networkFileData[index][4])));
            data.insert(std::pair<std::string, std::string>("inverse_bandwidth", std::to_string(networkFileData[index][2])));
            data.insert(std::pair<std::string, std::string>("comm_o_in_ns", std::to_string(networkFileData[index][5])));
            data.insert(std::pair<std::string, std::string>("comm_O_in_ns", std::to_string(networkFileData[index][7])));
            data.insert(std::pair<std::string, std::string>("comm_g_in_ns", std::to_string(networkFileData[index][6])));
            data.insert(std::pair<std::string, std::string>("comm_eagerlimit_in_bytes", std::to_string(networkFileData[index][3])));
        }
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
            if (!getKey(key))
                return defaultValue;
            return Conversion::stringTOScalarT<scalarT>(data.find(key)->second);
        }

    }; //!< end class configParser
} //!< end namespace useInterface
#endif //!< end CONFIGPARSER_HPP
