#summary This page provides a matrix showing which client works with which project.

The idea behind this page is to provide information for the case when changes at BOINC or synecdoche break the compatibility between (certain versions of) the synecdoche client and certain BOINC based projects.

### Project List ###
(For failure entries see the annotations section below for more information.)

| **Project**                   | **0.1.1**      | **0.2.0**      |
|:------------------------------|:---------------|:---------------|
| ABC@home                    | **OK**         | **OK**         |
| AQUA@home                   | **FAILED** (2) | **OK**         |
| Climate Prediction          | **OK**         | **OK**         |
| Cosmology@Home              | _not tested_ | _not tested_ |
| Docking@Home                | _not tested_ | _not tested_ |
| Einstein@home               | **OK**         | **OK**         |
| GPUGRID                     | _not tested_ | _not tested_ |
| Leiden Classical            | **OK**         | **OK**         |
| LHC@home                    | **OK**         | **OK**         |
| malariacontrol.net          | **OK**         | **OK**         |
| MilkyWay@home              | **OK**         | **OK**         |
| NQueens@Home Project        | **OK**         | **OK**         |
| POEM@HOME                   | **FAILED** (1) | **FAILED** (1) |
| PrimeGrid                  | **FAILED** (3) | **OK**         |
| QMC@Home                    | **OK**         | **OK**         |
| Rectilinear Crossing Number | **OK**         | **OK**         |
| Rosetta@home                | **OK**         | **OK**         |
| SETI@home                   | **OK**         | **OK**         |
| SHA-1 Collision Search Graz | **OK**         | **OK**         |
| SIMAP                       | **OK**         | **OK**         |
| Spinhenge@home              | **OK**         | **OK**         |
| Sudoku Project              | _not tested_ | _not tested_ |
| World Community Grid        | **OK**         | **OK**         |
| yoyo@home                   | _not tested_ | _not tested_ |
| µFluids                     | _not tested_ | _not tested_ |

### Annotations ###
  1. The POEM@home application modifies its input files when writing a checkpoint. From [r725](https://code.google.com/p/synecdoche/source/detail?r=725) on synecdoche checks if all input files are OK before starting the corresponding science application. The result is that the POEM input files are invalid from the client's perspective when restarting the POEM application after it wrote at least one checkpoint. As a safety measure the client aborts this workunit in this case. (Note: It can't re-download the file as this would destroy the state of the workunit.) This problem has been [reported to POEM@Home admins](http://boinc.fzk.de/poem/forum_thread.php?id=322) (German).
  1. Failed due to [issue 59](https://code.google.com/p/synecdoche/issues/detail?id=59).
  1. AP26 failed due to [issue 59](https://code.google.com/p/synecdoche/issues/detail?id=59), the other subprojects should work.