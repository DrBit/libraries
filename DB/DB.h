/*
  DB.h
  Database library for Arduino 
  Written by Madhusudana das
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DB_PROM
#define DB_PROM

#include "EEPROM.h"

struct DB_Header
{
  unsigned int n_recs;
  byte rec_size;
};

// slightly padded for the time being
#define DB_HEAD_SIZE 4

// DB_error values
#define DB_OK 0
#define DB_RECNO_OUT_OF_RANGE 1

#define DB_REC (byte*)(void*)&

typedef byte* DB_Rec;

class DB {
  public:
    void    create(unsigned int head_ptr, byte recsize, unsigned int init_size);
    void    open(unsigned int head_ptr);
    boolean write(unsigned int recno, const DB_Rec rec);
    boolean read(unsigned int recno, DB_Rec rec);
    boolean deleteRec(unsigned int recno);	                // delete is a reserved word
    boolean insert(unsigned int recno, const DB_Rec rec);
    void    append(DB_Rec rec);
    unsigned int   nRecs();
    DB_Header DB_head;
    byte DB_error;
  private:
    unsigned int writeHead();
    unsigned int readHead();
    unsigned int EEPROM_dbWrite(unsigned int ee, const byte* p);
    unsigned int EEPROM_dbRead(unsigned int ee, byte* p);
    unsigned int DB_head_ptr;
    unsigned int DB_tbl_ptr;
};

extern DB db;

#endif