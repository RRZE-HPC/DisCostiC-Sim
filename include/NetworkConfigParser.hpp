///    \file
///    NETWORKCONFIGPARSER_HPP
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
#include <climits>
#include <cstdlib>
#include <mpi.h>
#include "macro.hpp"
#include "DataStruct.hpp"

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
        double networkFileData[1000][8] = {0};
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

        void setData(const int numberOfConcurrentConnection, const int buffersize, DisCosTiC::Networktype &latency, DisCosTiC::Networktype &inverse_bandwidth, DisCosTiC::Networktype &o, DisCosTiC::Networktype &O, DisCosTiC::Networktype &g, DisCosTiC::Networktype &G, DisCosTiC_Datatype &eagerLimit, const int mode)
        {
            int index = 0;
            int bestIndex = 0;
            int minDiff = INT_MAX;
            bool foundConnectionMatch = false;

            //find entries matching the connection count and get the closest message size
            for (int i = 0; i < dataCounter; ++i)
            {
                if (networkFileData[i][0] >= numberOfConcurrentConnection)
                {
                    foundConnectionMatch = true;

                    //exact match for message size
                    if (networkFileData[i][1] == buffersize)
                    {
                        index = i;
                        break;
                    }

                    //track closest message size
                    int diff = abs((int)(networkFileData[i][1] - buffersize));
                    if (diff < minDiff)
                    {
                        minDiff = diff;
                        bestIndex = i;
                    }
                }
            }

            //use closest match if no exact match found
            if (!foundConnectionMatch)
            {
                //no connection match. use first entry
                index = 0;
            }
            else if (networkFileData[index][1] != buffersize)
            {
                // no exact match, use closest message size
                index = bestIndex;
            }

            latency[mode] = networkFileData[index][2];
            inverse_bandwidth[mode] = networkFileData[index][3];
            eagerLimit = networkFileData[index][4];
            G[mode] = networkFileData[index][5];
            o[mode] = networkFileData[index][6];
            g[mode] = networkFileData[index][7];
            O[mode] = networkFileData[index][8];

            // data.insert(std::pair<std::string, std::string>("latency_in_ns", std::to_string(networkFileData[index][2])));
            // data.insert(std::pair<std::string, std::string>("G_in_ns", std::to_string(networkFileData[index][3])));
            // data.insert(std::pair<std::string, std::string>("inverse_bandwidth", std::to_string(networkFileData[index][3])));
            // data.insert(std::pair<std::string, std::string>("comm_o_in_ns", std::to_string(networkFileData[index][5])));
            // data.insert(std::pair<std::string, std::string>("comm_O_in_ns", std::to_string(networkFileData[index][7])));
            // data.insert(std::pair<std::string, std::string>("comm_g_in_ns", std::to_string(networkFileData[index][6])));
            // data.insert(std::pair<std::string, std::string>("comm_eagerlimit_in_bytes", std::to_string(networkFileData[index][4])));
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
