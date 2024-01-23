
#ifndef TIMETRANKOP_HPP
#define TIMETRANKOP_HPP

#pragma once

//#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "json.hpp"				//!< JSON for Modern C++ version 2.1.1
using json = nlohmann::json;

#include "../include/ConfigParser.hpp"     //!< configuration file parser

namespace UserInterface {
    
    class TimeRankOP {
        /// private variables
    private:
        std::string content,filename;
        
        /**
         * \brief print number of processes to output file
         * \param numranks (total number of process)
         */
        inline void ranknum(DisCosTiC_Datatype numranks) {
            std::stringstream os;
            os << "numranks " << numranks << ";" << std::endl;
            AppendString(os.str(),content);
        }
        
        /**
         * \brief print all events (i.e., osend, orecv, execution, msg transmission) to output file
         * \param append
         */
        inline void file_write(bool append) {
            std::ofstream myfile;
            if (append)
                myfile.open(filename.c_str(), std::ios::out | std::ios::app);
            else
                myfile.open(filename.c_str(), std::ios::out);
            
            if (myfile.is_open()) {
                myfile << this->content;
                myfile.close();
            }
            else
                std::cerr << "error: missing result file definition of " << this->filename  << " for writing" << std::endl;
        }
        
        
    public:
        
        /**
         * \brief constructor that initializes the coordinates
         * \param CFG_args and number of ranks
         */
        TimeRankOP(UserInterface::ConfigParser *args_info, DisCosTiC_Datatype rank, int totalrank) {
            filename = args_info->getValue<std::string>("vizfilename") + std::to_string(rank) + ".out";
            ranknum(totalrank);
        }
        
        /**
         * \brief print time taken by overhead at sender side to output file
         * \param rank (process number), start (time), end (time), cpu (core number)
         */
        inline void osend(DisCosTiC_Datatype rank, DisCosTiC_Datatype start, DisCosTiC_Datatype end, DisCosTiC_Datatype cpu, float r=0.0, float g=0.0, float b=1.0) {	\
            std::stringstream os;			\
            os << "osend " << rank 			\
            << " " << cpu 					\
            << " " << start 				\
            << " " << end 					\
            << " " << r 					\
            << " " << g 					\
            << " " << b 					\
            << ";" << std::endl;			\
            AppendString(os.str(),content); \
        }
        
        /**
         * \brief print time taken by overhead at receiver side to output file
         * \param rank (process number), start (time), end (time), cpu (core number)
         */
        inline void orecv(DisCosTiC_Datatype rank, DisCosTiC_Datatype start, DisCosTiC_Datatype end, DisCosTiC_Datatype cpu, float r=0.0, float g=0.0, float b=1.0) {	\
            std::stringstream os;			\
            os << "orecv " << rank 			\
            << " " << cpu 					\
            << " " << start 				\
            << " " << end 					\
            << " " << r 					\
            << " " << g 					\
            << " " << b 					\
            << ";" << std::endl;			\
            AppendString(os.str(),content); \
        }
        
        /**
         * \brief print time taken by execution to output file
         * \param rank (process number), start (time), end (time), cpu (core number)
         */
        inline void comp(DisCosTiC_Datatype rank, DisCosTiC_Datatype start, DisCosTiC_Datatype end, DisCosTiC_Datatype cpu, float r=1.0, float g=0.0, float b=0.0) {	\
            std::stringstream os;			\
            os << "loclop " << rank 		\
            << " " << cpu 					\
            << " " << start 				\
            << " " << end 					\
            << " " << r 					\
            << " " << g 					\
            << " " << b						\
            << ";" << std::endl;			\
            AppendString(os.str(),content);	\
        }
        
        /**
         * \brief print time taken by message transmission to output file
         * \param rank (process number), start (time), end (time), cpu (core number)
         */
        inline void msg(DisCosTiC_Datatype source, DisCosTiC_Datatype dest, DisCosTiC_Datatype starttime, DisCosTiC_Datatype endtime, DisCosTiC_Datatype size, DisCosTiC_Datatype G, float r=0.0, float g=0.0, float b=1.0) {	\
            std::stringstream os;			\
            os << "transmission " << source \
            << " " << dest 					\
            << " " << starttime 			\
            << " " << endtime 				\
            << " " << size 					\
            << " " << G 					\
            << " " << r 					\
            << " " << g 					\
            << " " << b						\
            << ";" << std::endl;			\
            AppendString(os.str(),content);	\
        }
        
        /**
         * \brief destructor
         */
        ~TimeRankOP() {
            //file_write(false);
        }
    };      //!< end class TimeRankOP
    

	class ChromeTraceViz  {

    private:
        std::ofstream ofs;
        std::string filename;
        DisCosTiC_Indextype max_rank_id = 0;
        DisCosTiC_Indextype max_tid = 0;
        DisCosTiC_Indextype numRanks = 0;
        DisCosTiC_Indextype rank = -1;


    public:    
        std::map<std::string, DisCosTiC_Indextype> arc;

     	ChromeTraceViz(UserInterface::ConfigParser *args_info, DisCosTiC_Datatype rank, DisCosTiC_Datatype numRanks)
     	{
            if(rank != 0)
            {
                this->filename = args_info->getValue<std::string>("chromevizfilename") + std::to_string(rank - 1) + ".dms"; 
                ofs.open(filename);
                //ofs << "{ \"traceEvents\": [\n";
                this->numRanks = numRanks;
                this->rank = rank-1;
                if((rank - 1) == 0)
                {
                    ofs << "{ \"traceEvents\": [\n";

                    durationEventBegin("dummy",0,0,std::to_string(0));
                    durationEventEnd("dummy",0,0,std::to_string(0));
                }
            }
        }
        
            inline void args(std::string args_name,DisCosTiC_Indextype rank){
            auto it = arc.find(args_name);
            if (it == arc.end()){
              arc[args_name] = max_tid++;
            }
            max_rank_id = std::max(max_rank_id,rank);

        }

        inline void completeEvents(std::string name, 
        							DisCosTiC_Indextype rank, 
        							DisCosTiC_Indextype tid, 
        							std::string time,
        							std::string duration ){
            json js;
            js["name"] = name;
            js["ph"] = "X";
            js["dur"] = duration;
            js["pid"] = rank;
            js["tid"] = tid;
            js["ts"] = json::parse(time);
			ofs << std::setw(4)
             	<< js 
             	<< ","
             	<< std::endl;        
        }

        inline void durationEventBegin(std::string name, 
        								DisCosTiC_Indextype rank, 
        								DisCosTiC_Indextype tid, 
        								std::string time)
        								{
            json js;
            js["name"] = name;
            js["ph"] = "B";
            js["pid"] = rank;
            js["tid"] = tid;
            js["ts"] = json::parse(time);
			ofs << std::setw(4)
             	<< js 
             	<< ","
             	<< std::endl;        
        }

        inline void durationEventEnd(	std::string name, 
        								DisCosTiC_Indextype rank, 
        								DisCosTiC_Indextype tid, 
        								std::string time)
        								{
          json js;
            js["name"] = name;
            js["ph"] = "E";
            js["pid"] = rank;
            js["tid"] = tid;
            js["ts"] = json::parse(time);
            ofs << std::setw(4)
             	<< js 
             	<< ","
             	<< std::endl;
        }


        inline void flowEventBegin(std::string name, 
        							DisCosTiC_Indextype source, 
        							DisCosTiC_Indextype source_tid, 
        							std::string time, 
        							DisCosTiC_Indextype index)
        							{
            json js;
            js["name"] = name;
            js["ph"] = "s";
            js["id"] = index;
            js["pid"] = source;
            js["tid"] = source_tid;
            js["ts"] = json::parse(time);
            ofs << std::setw(4) 
            	<< js 
            	<< ","
            	<< std::endl;
        }


        inline void flowEventEnd(	std::string name, 
        							DisCosTiC_Indextype dest, 
        							DisCosTiC_Indextype dest_tid, 
        							std::string time, 
        							DisCosTiC_Indextype index)
        							{
            json js;
            js["name"] = name;
            js["ph"] = "f";
            js["id"] = index;
            js["pid"] = dest;
            js["tid"] = dest_tid;
            js["ts"] = json::parse(time);
            ofs << std::setw(4) 
            	<< js 
            	<< ","
            	<< std::endl;
        }

        inline void closeFile()
        {
            if(ofs.is_open() && this->rank != -1)// && (rank == (max_rank_id - 1)))
            {
                json j2 = {
                  {"name", "thread_name"},
                  {"ph", "M"},
                  {"pid", 0},
                  {"tid", 0},
                  {"args", {
                    {"name", "CPU"}
                  }}
        		};

                for(DisCosTiC_Indextype rank = 0; rank <= max_rank_id; rank++)
                {
                    j2["pid"] = rank;
                    for (const auto &p : this->arc) 
                    {
                        j2["tid"] = p.second;
                        j2["args"]["name"] = p.first;
                        ofs<< std::setw(4) << j2 << ","<<std::endl;
                    }
                   
                }
                
                /**
         		 * \brief print metadata for trace to output file
         		 */

                ofs.close();
                // if(this->rank == (this->numRanks-1))
                // {
                //     ofs << "{}\n],"
                //             << "\"Name\": \"Trace visualisation ofs DisCosTiC toolkit\", \n "
                //             << "\"Full form\": \"Distributed CosT in Cluster\", \n "
                //             << "\"Version\": \"v0.0.0 (initial release)\", \n "
                //             << "\"Author\": \"Ayesha Afzal <ayesha.afzal@fau.de>\", \n "
                //             << "\"Copyright\": \"© 2019 HPC, FAU Erlangen-Nuremberg. All rights reserved\" \n "
                //             << "}\n";
                    
                // }
            }
            
            
        }
        ~ChromeTraceViz()
        {}
        
 
	};  //!< end class TimeRankOP
	
    
}       //!< end namespace UserInterface
#endif  //!< end TIMETRANKOP_HPP
