//==================================================================================================
//  Radio.cpp is the Radio main program file for Radio2csv.
//
//  Author:  Copyright (c) 2007-2017 by Dean K. Gibson.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, version 2 of the License.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "Dstar.hpp"

char         const        version[]         = "Radio2csv v0.30 copyright (c) 2007-2017 by Dean Gibson/AE7Q\n";

double       const Radio::ctcssCodes[  50 ] = { 67.0,  69.3,  71.9,  74.4,  77.0,  79.7,  82.5,  85.4,  88.5,  91.5,
                                                94.8,  97.4, 100.0, 103.5, 107.2, 110.9, 114.8, 118.8, 123.0, 127.3,
                                               131.8, 136.5, 141.3, 146.2, 151.4, 156.7, 159.8, 162.2, 165.5, 167.9,
                                               171.3, 173.8, 177.3, 179.9, 183.5, 186.2, 189.9, 192.8, 196.6, 199.5,
                                               203.5, 206.5, 210.7, 218.1, 225.7, 229.1, 233.6, 241.8, 250.3, 254.1 };
uint16_t     const Radio::dcsCodes[   104 ] = {   23,    25,    26,    31,    32,    36,    43,    47,    51,    53,  
                                                  54,    65,    71,    72,    73,    74,   114,   115,   116,   122,  
                                                 125,   131,   132,   134,   143,   145,   152,   155,   156,   162,  
                                                 165,   172,   174,   205,   212,   223,   225,   226,   243,   244,  
                                                 245,   246,   251,   252,   255,   261,   263,   265,   266,   271,  
                                                 274,   306,   311,   315,   325,   331,   332,   343,   346,   351,  
                                                 356,   364,   365,   371,   411,   412,   413,   423,   431,   432,  
                                                 445,   446,   452,   454,   455,   462,   464,   465,   466,   503,  
                                                 506,   516,   523,   526,   532,   546,   565,   606,   612,   624,  
                                                 627,   631,   632,   654,   662,   664,   703,   712,   723,   731,  
                                                 732,   734,   743,   754 };
uint32_t     const Radio::divisorsX3[   5 ] = { 15000, 18750, 25000, 25000, 27000 };
char const * const Radio::dcsReverses[  4 ] = { "BOTH N", "TN-RR", "TR-RN", "BOTH R" };
char const * const Radio::skipModes[    3 ] = { "OFF", "Skip", "PSkip" };
char const * const Radio::splits[       4 ] = { "OFF", "DUP-", "DUP+", "RPS" };
char const * const Dstar::dvSquelches[  3 ] = { "OFF", "DSQL", "CSQL" };
char const * const Dstar::fmSquelches[ 12 ] = { "OFF", "TONE", "TSQL(*)", "TSQL", "DTCS(*)", "DTCS", "TSQL-R", "DTCS-R",
                                                "DTCS(T)", "TONE(T)/DTCS(R)", "DTCS(T)/TSQL(R)", "TONE(T)/TSQL(R)"  };
double       const Dstar::tuneSteps[   14 ] = {   5.0,   6.25,   8.33,    9.0,   10.0,   12.5,     15.0,     20.0,
                                                 25.0,   30.0,   50.0,  100.0,  125.0,  200.0 };

Routing const Routing::cqcqcq = "CQCQCQ";
Routing const Routing::direct = "DIRECT";
Routing const Routing::notUse = "";

typedef Radio * new_t( char const * pHeader, uint8_t const * pData, size_t address );
new_t   newId1,
        newId800,
        newIc91d,
        newIc92d,
        newIc80d,
        newId880,
        newIc2820,
        newId31,
        newId51,
        newId51p,
        newId51p2,
        newId5100,
        newIc7300,
        newThD74,
      * binModels[] = { newIc7300, newThD74 },
      * icfModels[] = { newId1,    newId800, newIc91d, newIc92d, newIc80d,  newId880,
                        newIc2820, newId31,  newId51,  newId51p, newId51p2, newId5100 };

char * parse( char ** ppLine ) {
  char * pResult,
       * pTemp;

  switch ( **ppLine ) {
  case 0:
    return *ppLine;
  case '"':
    pResult = ++*ppLine;
    for ( pTemp = pResult;  **ppLine != 0;  ++*ppLine ) {
      switch ( **ppLine ) {
        case '\r':
        case '\n':
        case '"':
          ++*ppLine;
          break;
        case '\\':
          ++*ppLine;
        default:  // fall-thru
          *pTemp++ = **ppLine;
          continue;
      }
      break;
    }
    *pTemp = 0;
    break;
  default:
//  *ppLine += strspn( *ppLine, " " );
    pResult = *ppLine;
    break;
  }
  *ppLine += strcspn( *ppLine, ",\r\n" );
  if ( **ppLine != 0 ) {
    *(*ppLine)++ = 0;
  }
  return pResult;
}

int stricmp( char const * pLhs, char const * pRhs ) {
  for( ;  *pLhs  &&  tolower( *pLhs ) == tolower( *pRhs );  pLhs++, pRhs++ );
  return *pLhs - *pRhs;
}

int search( char   const         * pKey,
            char   const * const * pTable,
            size_t                 count ) {

  for ( size_t index = 0;  index < count;  index++ ) {
    if ( stricmp( pKey, pTable[ index ] ) == 0 ) {
      return index;
    }
  }
  return -1;
}

int search( uint16_t         key,
            uint16_t const * pTable,
            size_t           count ) {

  for ( size_t index = 0;  index < count;  index++ ) {
    if ( key == pTable[ index ] ) {
      return index;
    }
  }
  return -1;
}

int search( double         key,
            double const * pTable,
            size_t         count,
            double         precision ) {
            
  for ( size_t index = 0;  index < count;  index++ ) {
    if (   key - pTable[ index ] < +precision
        && key - pTable[ index ] > -precision ) {
      return index;
    }
  }
  return -1;
}

int search( uint32_t         hz,
            uint32_t const * pTable,
            size_t           count ) {

  for ( size_t index = 0;  index < count;  index++ ) {
    if ( hz % pTable[ index ] == 0 ) {
      return index;
    }
  }
  return -1;
}

bool escape( char       * pExport,
             char const * pName,
             size_t       count ) {

  for ( *pExport++ = '"';  count-- > 0;  pName++ ) {
    switch ( *pName ) {
      case 0:
        break;  // exit loop
      case '"':
      case '\\':
        *pExport++ = '\\';
      default:  // fall-thru
        *pExport++ = *pName;
        continue;
    }
    break;
  }
  *pExport++ = '"';
  *pExport   = 0;
  return true;
}

void strpad( char       * pTarget,
             size_t       count,
             char const * pSource,
             char         pad ) {

  while ( count-- > 0 ) {
    *pTarget++ = *pSource ? *pSource++ : pad;
  }
}

enum {
  maxBytesPerLine = 0x10
};

CsvField const Dstar::csvFields[]
    = { { "CH No",           reinterpret_cast< getField_t >( & Dstar::_getValid        ),
                             reinterpret_cast< setField_t >( & Dstar::_setValid        ) },
        { "Frequency",       reinterpret_cast< getField_t >( & Dstar::_getRxFreq       ),
                             reinterpret_cast< setField_t >( & Dstar::_setRxFreq       ) },
        { "Dup",             reinterpret_cast< getField_t >( & Dstar::_getSplit        ),
                             reinterpret_cast< setField_t >( & Dstar::_setSplit        ) },
        { "Offset",          reinterpret_cast< getField_t >( & Dstar::_getTxOffset     ),
                             reinterpret_cast< setField_t >( & Dstar::_setTxOffset     ) },
        { "TS",              reinterpret_cast< getField_t >( & Dstar::_getRxStep       ),
                             reinterpret_cast< setField_t >( & Dstar::_setRxStep       ) },
        { "Mode",            reinterpret_cast< getField_t >( & Dstar::_getModulation   ),
                             reinterpret_cast< setField_t >( & Dstar::_setModulation   ) },
        { "Name",            reinterpret_cast< getField_t >( & Dstar::_getName         ),
                             reinterpret_cast< setField_t >( & Dstar::_setName         ) },
        { "SKIP",            reinterpret_cast< getField_t >( & Dstar::_getSkipMode     ),
                             reinterpret_cast< setField_t >( & Dstar::_setSkipMode     ) },
        { "TONE",            reinterpret_cast< getField_t >( & Dstar::_getFmSquelch    ),
                             reinterpret_cast< setField_t >( & Dstar::_setFmSquelch    ) },
        { "Repeater Tone",   reinterpret_cast< getField_t >( & Dstar::_getCtcssEncode  ),
                             reinterpret_cast< setField_t >( & Dstar::_setCtcssEncode  ) },
        { "TSQL Frequency",  reinterpret_cast< getField_t >( & Dstar::_getCtcssDecode  ),
                             reinterpret_cast< setField_t >( & Dstar::_setCtcssDecode  ) },
        { "DTCS Code",       reinterpret_cast< getField_t >( & Dstar::_getDcsCode      ),
                             reinterpret_cast< setField_t >( & Dstar::_setDcsCode      ) },
        { "DTCS Polarity",   reinterpret_cast< getField_t >( & Dstar::_getDcsReverse   ),
                             reinterpret_cast< setField_t >( & Dstar::_setDcsReverse   ) },
        { "DV SQL",          reinterpret_cast< getField_t >( & Dstar::_getDvSquelch    ),
                             reinterpret_cast< setField_t >( & Dstar::_setDvSquelch    ) },
        { "DV CSQL Code",    reinterpret_cast< getField_t >( & Dstar::_getDvCsqlCode   ),
                             reinterpret_cast< setField_t >( & Dstar::_setDvCsqlCode   ) },
        { "Your Call Sign",  reinterpret_cast< getField_t >( & Dstar::_getYourCall     ),
                             reinterpret_cast< setField_t >( & Dstar::_setYourCall     ) },
        { "RPT1 Call Sign",  reinterpret_cast< getField_t >( & Dstar::_getRpt1Call     ),
                             reinterpret_cast< setField_t >( & Dstar::_setRpt1Call     ) },
        { "RPT2 Call Sign",  reinterpret_cast< getField_t >( & Dstar::_getRpt2Call     ),
                             reinterpret_cast< setField_t >( & Dstar::_setRpt2Call     ) },
        { "Bank Group",      reinterpret_cast< getField_t >( & Dstar::_getBankGroup    ),
                             reinterpret_cast< setField_t >( & Dstar::_setBankGroup    ) },
        { "Bank Channel",    reinterpret_cast< getField_t >( & Dstar::_getBankChannel  ),
                             reinterpret_cast< setField_t >( & Dstar::_setBankChannel  ) },
        { 0, 0, 0 } };

bool Radio::load( FILE * pFile ) {

  CsvField const * const csvField = csvHeader();
  char line[ 1024 ];
  if ( fgets( line, sizeof line, pFile ) == 0 ) {
    fprintf( stderr, "*** Missing CSV data ***\n" );
    return false;
  }
  char * pNext = line;
  size_t csvFieldCount = 0;
  size_t csvOrder[ 256 ];
  while ( *pNext != 0  &&  csvField[ csvFieldCount ].fieldName != 0  &&  csvFieldCount < COUNT_OF( csvOrder ) ) {
    char * pTemp = parse( & pNext );
    size_t fieldIndex = 0;
    for (  ;  csvField[ fieldIndex ].fieldName != 0  ;  fieldIndex++ ) {
      if ( stricmp( pTemp, csvField[ fieldIndex ].fieldName ) == 0 ) {
        csvOrder[ csvFieldCount++ ] = fieldIndex;
        break;
      }
    }
    if ( csvField[ fieldIndex ].fieldName == 0 ) {
      fprintf( stderr, "*** Unknown CSV header field name '%s' ***\n", pTemp );
      return false;
    }
  }
  if ( csvOrder[ 0 ] != 0 ) {
    fprintf( stderr, "*** First CSV header field name '%s' is not the channel number ***\n",
                     csvField[ csvOrder[ 0 ] ].fieldName );
    return false;
  }

  size_t count = 0;
  size_t offset = getOffset();
  while ( fgets( line, sizeof line, pFile ) != 0 ) {
    pNext = line;
    char * pTemp;
    size_t lineIndex = strtoul( parse( & pNext ), & pTemp, 10 ) - offset;
    if ( *pTemp != 0  || !(this->*csvField[ 0 ].setField)( lineIndex, pNext ) ) {
      fprintf( stderr, "*** Channel %d: Invalid number; line skipped ***\n", (int)(lineIndex + offset) );
      continue;
    }
    if ( *pNext != 0 ) {
      count++;
      for ( size_t fieldIndex = 1;  fieldIndex < csvFieldCount;  fieldIndex++ ) {
        char * pTemp = parse( & pNext );
        if ( !(this->*csvField[ csvOrder[ fieldIndex ] ].setField)( lineIndex, pTemp ) ) {
          fprintf( stderr, "*** Channel %d, field '%s': Invalid field contents '%s'; line skipped ***\n",
                           (int)(lineIndex + offset), csvField[ csvOrder[  fieldIndex ] ].fieldName, pTemp );
          (this->*csvField[ 0 ].setField)( lineIndex, 0 );
          count--;
          break;
        }
      }
    }
  }
  fprintf( stderr, "--- Lines loaded: %d ---\n", (int)count );
  return true;
}

void Radio::dump( FILE * pFile ) const {
  CsvField const * const csvField = csvHeader();
  size_t csvFieldCount = 0;
  for (  ;  csvField[ csvFieldCount ].fieldName != 0;  csvFieldCount++ ) {
    if ( csvFieldCount > 0 ) {
      fputc( ',', pFile );
    }
    fprintf( pFile, "%s", csvField[ csvFieldCount ].fieldName );
  }
  fputc( '\n', pFile );
  size_t offset = getOffset();
  for ( size_t lineIndex = 0;  lineIndex < getCount();  lineIndex++ ) {
    if ( (this->*csvField[ 0 ].getField)( lineIndex, 0 ) ) {
      char work[ 1024 ];
      sprintf( work, "%d", (int)(lineIndex + offset) );
      for ( size_t fieldIndex = 1;  fieldIndex < csvFieldCount;  fieldIndex++ ) {
        fprintf( pFile, "%s,", work );
        if ( !(this->*csvField[ fieldIndex ].getField)( lineIndex, work ) ) {
          fprintf( stderr, "*** Channel %d, field '%s': Unknown field value '%s' ***\n",
                           (int)(lineIndex + offset), csvField[ fieldIndex ].fieldName, work );
        }
      }
      fprintf( pFile, "%s\n", work );
    }
  }
}

void Radio::save( FILE * pFile, bool isBinary, char const * pComment ) {
  char const * pModel = setComment( pComment );
  if ( isBinary ) {
    fwrite( pData, 1, size, pFile );
  } else {
    fputs( pHeader, pFile );
    size_t maxLength = size > 0x10000 ? 0x20 : 0x10;
    for ( size_t address = 0;  address < size;  ) {
      size_t length = size - address < maxLength
                    ? size - address : maxLength;
      fprintf( pFile, size > 0x10000 ? "%08X%02X" : "%04X%02X", (int)address, (int)length );
      for ( size_t index = 0;  index < length;  index++ ) {
        fprintf( pFile, "%02X", pData[ address++ ] & UCHAR_MAX );
      }
      fputc( '\n', pFile );
    }
  }
  if ( pComment == 0 ) {
    fprintf( stderr, "--- %s updated ---\n", pModel );
  } else {
    fprintf( stderr, "--- %s ('%s') updated ---\n", pModel, pComment );
  }
}

Radio * Radio::create( FILE * pFile, bool isBinary ) {
  new_t     ** models;
  uint8_t      memory[ 1048576 ];
  char         header[ 1000 ],
               work[ 256 ];
  size_t       address = 0,
               count;

  if ( isBinary ) {
    address = fread( memory, 1, sizeof memory, pFile );
    models = binModels;
    count = COUNT_OF( binModels );
  } else {
    char line[ 1024 ];
    for ( size_t index = 0;  fgets( line, sizeof line, pFile );  index += strlen( line ) ) {
      if ( index > 0  &&  line[ 0 ] != '#' ) {
        break;
      }
      if ( index > sizeof header - sizeof line ) {
        fprintf( stderr, "*** Invalid file format (file header size) *** \n" );
        return NULL;
      }
      strcpy( & header[ index ], line );
    }
    do {
      if ( line[ 0 ] != '#' ) {
        memcpy( work, line, 10 );
        size_t length = strlen( line );
        for ( ;  length > 0  &&  line[ length - 1 ] < ' ';  line[ --length ] = 0 ); 
        size_t index = length % 0x10;
        index = index < 6 ? 6 : index;  // special-case ID-1 last line
        work[ index ] = 0;
        char * pTemp;
        count    = strtoul( & work[ index - 2 ], & pTemp, 0x10 );
        work[ index - 2 ] = 0;
        size_t location = strtoul( & work[ 0  ], & pTemp, 0x10 );
        if ( location + count >= sizeof memory ) {
          fprintf( stderr, "*** Input file too large ***\n  %s", line );
          return NULL;
        }
        if ( length != index + count * 2
            ||  location != address
            ||  address + count >= sizeof memory ) {
          fprintf( stderr, "*** Invalid input file data line format ***\n  %s", line );
          fprintf( stderr, "\n  %d %d %d ", (int)length, (int)index, (int)count );
          return NULL;
        }
        work[ 2 ] = 0;
        while ( index < length ) {
          work[ 0 ] = line[ index++ ];
          work[ 1 ] = line[ index++ ];
          memory[ address++ ] = strtoul( work, & pTemp, 0x10 );
        }
      }
    } while ( fgets( line, sizeof line, pFile ) );
    models = icfModels;
    count = COUNT_OF( icfModels );
  }
  Radio      * pRadio;
  for ( size_t index = 0;  index < count;  index++ ) {
    pRadio = models[ index ]( header, memory, address );
    if ( pRadio ) {
      char const * pModel = pRadio->getComment( work );
      for ( size_t index = strlen( work );  index-- > 0  &&  work[ index ] == ' ';  work[ index ] = 0 );
      if ( work[ 0 ] == 0 ) {
        fprintf( stderr, "=== %s found ===\n", pModel );
      } else {
        fprintf( stderr, "=== %s ('%s') found ===\n", pModel, work );
      }
      return pRadio;
    }
  }
  fprintf( stderr, "*** File does not match any known radio ***\n" );
  return NULL;
}

int __cdecl main( int                argc,
                  char const * const argv[] ) {
  FILE       * pFile;
  Radio      * pRadio;
  char         Icf[]    = ".ICF";
  bool         isBinary = false;


  fprintf( stderr, version );
  switch( argc ) {
  case 1:
    fprintf( stderr, "  Usage:  Radio2csv  Radio-filein  [ Radio-fileout [ 'comment' ] ]\n"
                     "    To export frequency memories (\"Channels\") to a CSV file:\n"
                     "\tRadio2csv  Radio-file  > CSV-file\n"
                     "    To import frequency memories (\"Channels\") from a CSV file:\n"
                     "\tRadio2csv  Radio-oldfile  Radio-newfile  < CSV-file\n" );
    return 1;
  case 2:
  case 3:
  case 4:
    break;
  default:
     fprintf( stderr, "*** Parameter error;  too many parameters ***\n" );
     return 3;
  }
  pFile = fopen( argv[ 1 ], "rb" );
  if ( pFile == 0 ) {
    fprintf( stderr, "*** File not found: '%s' ***\n", argv[ 1 ] );
    return 2;
  }
  size_t offset = strlen( argv[ 1 ] ) - (sizeof Icf - 1);
  if ( (int)offset > 0 ) {
    for ( size_t index = 0;  index < sizeof Icf;  ) {
      isBinary |= toupper( argv[ 1 ][ offset++ ] ) != Icf[ index++ ];
    }
  }
  pRadio = Radio::create( pFile, isBinary );
  fclose( pFile );
  if ( pRadio == 0 ) {
    return 2;
  }
  if ( argv[ 2 ] != 0 ) {
    if ( ! pRadio->load( stdin ) ) {
      fprintf( stderr, "*** Unable to load file 'stdin' ***\n" );
      return 2;
    }
    pFile = fopen( argv[ 2 ], "wb" );
    if ( pFile == 0 ) {
      fprintf( stderr, "*** Unable to write file: '%s' ***\n", argv[ 2 ] );
      return 2;
    }
    pRadio->save( pFile, isBinary, argv[ 3 ] );
    fclose( pFile );
  } else {
    pRadio->dump( stdout );
  }
  delete pRadio;

  return 0;
}
