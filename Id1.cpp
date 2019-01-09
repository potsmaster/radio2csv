//==================================================================================================
//  Id1.cpp is the Icom ID-1 radio class file for Radio2csv.
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

static char const * const modulations[] = { "?0?",   "FM",  "DV",   "DD" };
static char const * const fmSquelches[] = { "OFF", "TONE", "?2?", "TSQL" };

struct Id1Channel {       // little-endian target
  FrequencyLE< 32 >     rxFreq;
  FrequencyLE< 32 >     txOffset;
  uint8_t LOW2HI_ORDER(              : 2,
          LOW2HI_ORDER( direction    : 2,
          LOW2HI_ORDER( fmSquelch    : 2,
                        modulation   : 2 )));
  uint8_t LOW2HI_ORDER(              : 1,
          LOW2HI_ORDER( dvSquelch    : 2,
          LOW2HI_ORDER(              : 4,
                        skipMode     : 1 )));
  uint8_t LOW2HI_ORDER( ctcssEncode  : 6,
                                     : 2 );
  uint8_t LOW2HI_ORDER( ctcssDecode  : 6,
                                     : 2 );
  Routing               rpt2Call;
  Routing               rpt1Call;
  Routing               yourCall;
  uint8_t LOW2HI_ORDER( dvCsqlCode   : 7,
                                     : 1 );
  uint8_t               zero;
  char                  name[ 10 ];
};

struct Id1Memory {
  enum {
    CHANNELS =  100,
    EDGES    =    1
  };
  
  Id1Channel channel[ CHANNELS ];  // 0x0000
  struct {
    Id1Channel lower,
               upper;
  }          scanEdge[ EDGES ];    // 0x12C0
  Id1Channel call[ 3 ];            // 0x1320
};                                 // 0x13B0

class Id1 : public Dstar {
  Id1Memory * pMemory;

  Id1(             void            );  // Intentionally not implemented
  Id1(             Id1 const & rhs );  // Intentionally not implemented
  Id1 & operator=( Id1 const & rhs );  // Intentionally not implemented

 protected:
  static CsvField const csvFields[];

  virtual CsvField const * csvHeader( void ) const {
    return csvFields;
  }

  virtual bool           getIgnore(      size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setIgnore(      size_t   index, bool         value   )       { assert( false ); return 0; }
  virtual bool           getSkip(        size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setSkip(        size_t   index, bool         value   )       { assert( false ); return 0; }
  virtual bool           getSkipp(       size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setSkipp(       size_t   index, bool         value   )       { assert( false ); return 0; }

  virtual bits_t         getRxStep(      size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setRxStep(      size_t   index, bits_t       value   )       { assert( false ); return 0; }
  virtual double const * getTuneSteps(   size_t * count                       ) const { assert( false ); return 0; }
  virtual bits_t         getDcsCode(     size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setDcsCode(     size_t   index, bits_t       value   )       { assert( false ); return 0; }
  virtual bits_t         getDcsReverse(  size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setDcsReverse(  size_t   index, bits_t       value   )       { assert( false ); return 0; }

  virtual bits_t         getBankGroup(   size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setBankGroup(   size_t   index, bits_t       value   )       { assert( false ); return 0; }
  virtual bits_t         getBankChannel( size_t   index                       ) const { assert( false ); return 0; }
  virtual bool           setBankChannel( size_t   index, bits_t       value   )       { assert( false ); return 0; }

  virtual char const         * getComment(                      char        * pExport ) const {
    *pExport = 0;
    return "Icom ID-1";
  }
  virtual char const         * setComment(                      char const  * pImport )       {
    return "Icom ID-1";
  }
  virtual size_t               getCount(         void                                 ) const {
    return Id1Memory::CHANNELS;
  }

  virtual bool                 getValid(         size_t   index                       ) const {
    return pMemory->channel[ index ].rxFreq.hz() > 0;
  }
  virtual void                 setValid(         size_t   index, bool         value   ) {
    memset( & pMemory->channel[ index ], value ? 0 : -1, sizeof pMemory->channel[ 0 ] );
    setName(    index, "" );
  }

  virtual uint32_t             getRxFreq(        size_t   index                       ) const {
    return pMemory->channel[ index ].rxFreq.hz();
  }
  virtual bool                 setRxFreq(        size_t   index, uint32_t     value   ) {
    pMemory->channel[ index ].rxFreq.set( value );
    return true;
  }

  virtual bits_t               getSplit(         size_t   index                       ) const {
    return pMemory->channel[ index ].direction;
  }
  virtual bool                 setSplit(         size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].direction = value;
    return true;
  }

  virtual uint32_t             getTxOffset(      size_t   index                       ) const {
    return pMemory->channel[ index ].txOffset.hz();
  }
  virtual bool                 setTxOffset(      size_t   index, uint32_t     value   ) {
    pMemory->channel[ index ].txOffset.set( value );
    return true;
  }

  virtual char const * const * getModulations(   size_t * count                       ) const {
    *count = COUNT_OF( ::modulations );
    return ::modulations;
  }
  virtual bits_t               getModulation(    size_t   index                       ) const {
    return pMemory->channel[ index ].modulation;
  }
  virtual bool                 setModulation(    size_t   index, bits_t        value  ) {
    pMemory->channel[ index ].modulation = value;
    return true;
  }

  virtual char const         * getName(          size_t   index, size_t      * pSize  ) const {
    *pSize = sizeof pMemory->channel[ 0 ].name;
    return pMemory->channel[ index ].name;
  }
  virtual bool                 setName(          size_t   index, char const * pValue  ) {
    strpad( pMemory->channel[ index ].name, sizeof pMemory->channel[ 0 ].name, pValue, ' ');
    return true;
  }

  virtual bits_t               getSkipMode(      size_t   index                       ) const {
    return pMemory->channel[ index ].skipMode;
  }
  virtual bool                 setSkipMode(      size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].skipMode = value;
    return true;
  }

  virtual char const * const * getFmSquelches(   size_t * count                       ) const {
    *count = COUNT_OF( ::fmSquelches );
    return ::fmSquelches;
  }
  virtual bits_t               getFmSquelch(     size_t   index                       ) const {
    return pMemory->channel[ index ].fmSquelch;
  }
  virtual bool                 setFmSquelch(     size_t   index, bits_t        value  ) {
    pMemory->channel[ index ].fmSquelch = value;
    return true;
  }

  virtual bits_t               getCtcssEncode(   size_t   index                       ) const {
    return pMemory->channel[ index ].ctcssEncode;
  }
  virtual bool                 setCtcssEncode(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].ctcssEncode = value;
    return true;
  }

  virtual bits_t               getCtcssDecode(   size_t   index                       ) const {
    return pMemory->channel[ index ].ctcssDecode;
  }
  virtual bool                 setCtcssDecode(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].ctcssDecode = value;
    return true;
  }

  virtual bits_t               getDvSquelch(     size_t   index                       ) const {
    return pMemory->channel[ index ].dvSquelch;
  }
  virtual bool                 setDvSquelch(     size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dvSquelch = value;
    return true;
  }

  virtual bits_t               getDvCsqlCode(    size_t   index                       ) const {
    return pMemory->channel[ index ].dvCsqlCode;
  }
  virtual bool                 setDvCsqlCode(    size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dvCsqlCode = value;
    return true;
  }

  virtual Routing              getYourCall(      size_t   index                       ) const {
    return pMemory->channel[ index ].yourCall;
  }
  virtual bool                 setYourCall(      size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].yourCall = pValue;
    return true;
  }

  virtual Routing              getRpt1Call(      size_t   index                       ) const {
    return pMemory->channel[ index ].rpt1Call;
  }
  virtual bool                 setRpt1Call(      size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].rpt1Call = pValue;
    return true;
  }

  virtual Routing              getRpt2Call(      size_t   index                       ) const {
    return pMemory->channel[ index ].rpt2Call;
  }
  virtual bool                 setRpt2Call(      size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].rpt2Call = pValue;
    return true;
  }

 public:
  Id1( char    const * pHeader,
       uint8_t const * pData,
       size_t          size )
      : Dstar( pHeader, pData, size ), pMemory( (Id1Memory *)Dstar::pData ) {
    assert( sizeof( Id1Channel ) == 48 );
  }

  virtual ~Id1( void ) {}
};

CsvField const Id1::csvFields[]
    = { { "CH No",           reinterpret_cast< getField_t >( & Id1::_getValid        ),
                             reinterpret_cast< setField_t >( & Id1::_setValid        ) },
        { "Frequency",       reinterpret_cast< getField_t >( & Id1::_getRxFreq       ),
                             reinterpret_cast< setField_t >( & Id1::_setRxFreq       ) },
        { "Dup",             reinterpret_cast< getField_t >( & Id1::_getSplit        ),
                             reinterpret_cast< setField_t >( & Id1::_setSplit        ) },
        { "Offset",          reinterpret_cast< getField_t >( & Id1::_getTxOffset     ),
                             reinterpret_cast< setField_t >( & Id1::_setTxOffset     ) },
        { "Mode",            reinterpret_cast< getField_t >( & Id1::_getModulation   ),
                             reinterpret_cast< setField_t >( & Id1::_setModulation   ) },
        { "Name",            reinterpret_cast< getField_t >( & Id1::_getName         ),
                             reinterpret_cast< setField_t >( & Id1::_setName         ) },
        { "SKIP",            reinterpret_cast< getField_t >( & Id1::_getSkipMode     ),
                             reinterpret_cast< setField_t >( & Id1::_setSkipMode     ) },
        { "TONE",            reinterpret_cast< getField_t >( & Id1::_getFmSquelch    ),
                             reinterpret_cast< setField_t >( & Id1::_setFmSquelch    ) },
        { "Repeater Tone",   reinterpret_cast< getField_t >( & Id1::_getCtcssEncode  ),
                             reinterpret_cast< setField_t >( & Id1::_setCtcssEncode  ) },
        { "TSQL Frequency",  reinterpret_cast< getField_t >( & Id1::_getCtcssDecode  ),
                             reinterpret_cast< setField_t >( & Id1::_setCtcssDecode  ) },
        { "DV SQL",          reinterpret_cast< getField_t >( & Id1::_getDvSquelch    ),
                             reinterpret_cast< setField_t >( & Id1::_setDvSquelch    ) },
        { "DV CSQL Code",    reinterpret_cast< getField_t >( & Id1::_getDvCsqlCode   ),
                             reinterpret_cast< setField_t >( & Id1::_setDvCsqlCode   ) },
        { "Your Call Sign",  reinterpret_cast< getField_t >( & Id1::_getYourCall     ),
                             reinterpret_cast< setField_t >( & Id1::_setYourCall     ) },
        { "RPT1 Call Sign",  reinterpret_cast< getField_t >( & Id1::_getRpt1Call     ),
                             reinterpret_cast< setField_t >( & Id1::_setRpt1Call     ) },
        { "RPT2 Call Sign",  reinterpret_cast< getField_t >( & Id1::_getRpt2Call     ),
                             reinterpret_cast< setField_t >( & Id1::_setRpt2Call     ) },
        { 0, 0, 0 } };

Dstar * newId1( char const * pHeader,
                     uint8_t const * pData,
                     size_t          size ) {
  return size == sizeof( Id1Memory ) - 1  &&  memcmp( pHeader, "25060000", 8 ) == 0
         ? new Id1( pHeader, pData, size ) : 0;
}
