// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

#define MAX_CPU_BENCHMARKS_SECONDS  300
#define CPU_BENCHMARKS_RUNNING      0
#define CPU_BENCHMARKS_COMPLETE     1
#define CPU_BENCHMARKS_NOT_RUNNING  2
#define CPU_BENCHMARKS_ERROR        3

#define BM_TYPE_FP       0
#define BM_TYPE_INT      1

int dhrystone(double& vax_mips, double& loops, double& cpu_time, double min_cpu_time);
int whetstone(double& flops, double& cpu_time, double min_cpu_time);
void benchmark_wait_to_start(int which);
bool benchmark_time_to_stop(int which);
