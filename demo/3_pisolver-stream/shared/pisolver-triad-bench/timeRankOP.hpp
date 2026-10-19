///    \file
///    TIMERANKOP_HPP
///
///    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///
/// Purpose: Optional per-rank timing file writer for pisolver-triad-bench.
///
#ifndef TIMETRANKOP_HPP
#define TIMETRANKOP_HPP

#pragma once

#include <sstream>
#include <fstream>

//#define OPFileName "timeRankViz.cvs"

//#ifndef OPFileName
//#error error: missing result file definition (OPFileName)
//#endif

namespace userInterface {
	class timeRankOP {

	private:
		std::string content;
		std::string filename;

    	void numRanks(int nRanks) {
    		std::stringstream oss;
        	oss << "numranks " << nRanks << ";\n";
        	this->content.append(oss.str());
    	}

    	void write(bool append) {
        	std::ofstream resultFile;
        	if (append) 
            	resultFile.open(filename, std::ios::out | std::ios::app);
        	else 
            	resultFile.open(filename, std::ios::out);
        
        	if (resultFile.is_open()) {
            	//myfile << "Writing this to a file.\n";
            	resultFile << this->content;
            	resultFile.close();
        	}
        	else {
            	std::cout << "error: missing result file definition so unable to open file " << filename << std::endl;
        	}
    	}

	public:

    	timeRankOP(int id, int myrank) {
//        	numRanks(id);
        	filename =  "timeRankViz" + std::to_string(myrank) + ".cvs";
//          filename =  "StrongScaling.cvs";
    	}
    	
    	void time_vtkStep(int rank, int timeStep, double time) {
  			std::stringstream oss;   
    		oss << std::fixed << std::setprecision(52)<< timeStep << " " << rank << " " << time << "\n";
    		this->content.append(oss.str());
  		}

  		void add_loclop(int rank, double start, double end, int cpu) {
  			std::stringstream oss;   
    		oss << "loclop " << rank << " " << cpu << " " << start << " " << end << ";\n";
    		this->content.append(oss.str());
      		//std::cout << "\nloclop " << rank << " " << cpu << " " << start << " " << end << std::endl;
  		}

  		void add_transmission(int source, int dest, double starttime, double endtime, int size) {
			std::stringstream oss;
    		oss << "transmission " << source << " " << dest << " " << starttime << " ";
    		oss << endtime << " " << size << ";\n";
    		this->content.append(oss.str());
  		}
  
      void add_totaltime(double exectime, double commtime, double totaltime) {
      std::stringstream oss;
        oss << exectime << " " << commtime << " " << totaltime;
        this->content.append(oss.str());
      }
	
    	~timeRankOP() {
        	write(false);
    	}
	}; 	//!< end class timeRankOP
}		//!< end namespace userInterface
#endif 	//!< end TIMETRANKOP_HPP


class timer
{
    public:
        timer() = default;
    
        void start(const std::string& Cout){
            text = Cout;
            begin = std::chrono::high_resolution_clock::now();
        }
    
        void stop(){
            end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << std::setw(30) << text << ":" << std::setw(5) << ms << "ms" << std::endl;
        }
    
    private:
        std::string text;
        std::chrono::high_resolution_clock::time_point begin;
        std::chrono::high_resolution_clock::time_point end;
};
