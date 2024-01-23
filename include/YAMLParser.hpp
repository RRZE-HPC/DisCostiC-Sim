///    \file
///    YAMLPARSER_HPP
///
///    \Author: Ayesha Afzal <ayesha.afzal@fau.de>
///    \Copyright © 2019 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///

#ifndef YAMLPARSER_HPP
#define YAMLPARSER_HPP

#pragma once

#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <typeinfo>
#include <mpi.h>
#include "macro.hpp"

/**
 *\brief it parses the user-defined configuration file (.yml)
 */
namespace UserInterface
{
    class YAMLParser
    {
    public:
        std::map<std::string, std::string> data; //!< which will hold pairs of key-value
        std::string fileName;                    //!< As member variables, we will only have a std::string, which will hold the name of the configuration file
        std::string micro_architecture;
        DisCosTiC_Datatype FP_instructions_per_cycle;
        DisCosTiC_Datatype FP_ops_per_instruction_SP;
        DisCosTiC_Datatype FP_ops_per_instruction_DP;
        real_t clk_freq_in_GHz;
        DisCosTiC_Datatype cores_per_chip, chips_per_node, cores_per_numa_domain;
        std::vector<real_t> MEM_bandwidth;
        bool flag;

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
         *\brief a function that extracts the key from the pair of key = value
         */

        /**
         *\brief a function that parse the line by calling above mentioed functions
         */
        void parseLine(const std::string &line, size_t const lineNum)
        {
            if (line.find("clock:") != std::string::npos)
                clk_freq_in_GHz = std::stod(line.substr(line.find(":") + 1, 4));

            if (line.find("cores per socket:") != std::string::npos)
                cores_per_chip = std::stoi(line.substr(line.find(":") + 1, 3));

            if (line.find("cores per NUMA domain:") != std::string::npos)
                cores_per_numa_domain = std::stoi(line.substr(line.find(":") + 1, 3));

            if (line.find("DP:") != std::string::npos)
                FP_ops_per_instruction_DP = std::stoi(line.substr(line.find("total:") + 6, 3));

            if (line.find("SP:") != std::string::npos)
                FP_ops_per_instruction_SP = std::stoi(line.substr(line.find("total:") + 6, 3));

            if (line.find("sockets:") != std::string::npos)
                chips_per_node = std::stoi(line.substr(line.find(":") + 1, 3));

            if (line.find(" MEM:") != std::string::npos)
            {
                flag = false;
            }
            if (flag == false && (line.find("copy:") != std::string::npos))
            {

                std::string val = line.substr(line.find("[") + 1, line.find("]") - 17);

                std::string a;
                for (std::stringstream sst(val); std::getline(sst, a, ',');)
                    MEM_bandwidth.push_back(stod(a.replace(a.find("GB/s"), 4, "")));
                flag = true;
            }
        }

        /**
         *\brief a class to set the name of the configuration file and extracts and parses the data
         */
        YAMLParser(const std::string &fileName)
        {

            std::ifstream file;
            fileopen(file, fileName);

            std::string line;
            size_t lineNum = 0;
            flag = true;

            micro_architecture = fileName.substr(24, fileName.substr(24, 50).find(".yml"));

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

    }; //!< end class YAMLParser
} //!< end namespace useInterface
#endif //!< end YAMLPARSER_HPP
