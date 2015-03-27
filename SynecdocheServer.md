# Motivation of this page #

This page should be used to collect and discuss ideas, plans, etc. for the synecdoche server. There will be no detailed roadmap or any dates yet and development probably won't start before there is at least one (stable) release of the synecdoche client.


# TODO-List #

  * Redesign the database abstraction layer to support more DBMSs.
  * Port the server software to windows.



# Detailed description of TODO-items #

## Database abstraction layer ##

The database "abstraction layer" used by BOINC server software is just a little wrapper around some MySQL functions and doesn't provide a level of abstraction which allows to exchange the DBMS. It should be re-designed to allow the use of different systems like for example PostgreSQL, Ingres, MSSQL and Oracle.

There are probably two ways for reaching this goal: Use ODBC drivers or provide a custom adapter for each DBMS. Using ODBC means less coding work as the ODBC driver already provides an abstraction of the DBMS. However, the ODBC interface might be somewhat limited and may be a disadvantage when talking about performance. Providing a custom adapter for each supported DBMS may not have these problems but requires a carefull design and more coding work.