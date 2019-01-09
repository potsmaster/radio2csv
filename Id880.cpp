//==================================================================================================
//  Id880.cpp is the Icom ID-880H & IC-80D radio class file for Radio2csv.
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

static char const * const modulations[ 8 ] = { "FM", "FM-N", "?2?", "AM", "AM-N", "DV", "?6?", "?7?" };

struct Id8X0Channel : public _IdDrChannel< 8 >{};  // big-endian target

struct Id8X0Memory {
  enum {
    CHANNELS = 1000,
    BANKS    =   26,
    EDGES    =   25
  };
  
  Id8X0Channel  channel[ CHANNELS ];  // 0x0000
  struct {
    Id8X0Channel  lower,
                  upper;
  }             scanEdge[ EDGES ];    // 0xA028
  Id8X0Channel  call[ 2 ];            // 0xA82A
  uint8_t       fill1[ 70*7 ];        // 0xA87C
  uint8_t       fill2[   26 ];        // 0xAA66
  uint8_t       ignoreChannel[ (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0xAA80
  uint8_t       skipChannel[   (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0xAB04
  uint8_t       skippChannel[  (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0xAB88
  uint8_t       fill3[  244 ];                                               // 0xAC0C
  BankMap       bankMap[        CHANNELS + EDGES * 2 ];                      // 0xAD00
  uint8_t       fill4[   12 ];        // 0xB534
  char          comment[ 16 ];        // 0xB540
  uint8_t       spaces[ 436 ];        // 0xB550
  uint8_t       fill5[  444 ];        // 0xB704
  uint8_t       fill6[ 9622 ];        // 0xB8C0
  MyCall        myCall[   6 ];        // 0xDE56
//Routing       urCall[  60 ];        // 0x????
//Routing       rpCall[  60 ];        // 0x????
  uint8_t       fill9[ 5986 ];        // 0xDE9E
};                                    // 0xF600

class Id8X0 : public Dstar {
  Id8X0Memory * pMemory;

  Id8X0(             void              );  // Intentionally not implemented
  Id8X0(             Id8X0 const & rhs );  // Intentionally not implemented
  Id8X0 & operator=( Id8X0 const & rhs );  // Intentionally not implemented

 protected:
  virtual bool               getIgnore(       size_t index                       ) const {
    return getBit( pMemory->ignoreChannel, index );
  }
  virtual bool               setIgnore(       size_t index, bool         value   )       {
    return setBit( pMemory->ignoreChannel, index, value );
  }
  virtual bool               getSkip(         size_t index                       ) const {
    return getBit( pMemory->skipChannel,   index );
  }
  virtual bool               setSkip(         size_t index, bool         value   )       {
    return setBit( pMemory->skipChannel,   index, value );
  }
  virtual bool               getSkipp(        size_t index                       ) const {
    return getBit( pMemory->skippChannel,  index );
  }
  virtual bool               setSkipp(        size_t index, bool         value   )       {
    return setBit( pMemory->skippChannel,  index, value );
  }

  virtual void              _getComment(                    char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
  }
  virtual void              _setComment(                    char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
  }
  virtual size_t             getCount(        void                               ) const {
    return Id8X0Memory::CHANNELS;
  }

  virtual bool               getValid(        size_t index                       ) const {
    return !getIgnore( index );
  }
  virtual void               setValid(        size_t index, bool         value   ) {
    memset( & pMemory->channel[ index ], value ? 0 : -1, sizeof pMemory->channel[ 0 ] );
    memset( & pMemory->bankMap[ index ], value ? 0 : -1, sizeof pMemory->bankMap[ 0 ] );
    pMemory->channel[ index ].unknown1 = 0xE4;
    setName(     index, "" );
    setSkipMode( index, 0 );
    setIgnore(   index, !value );
  }
  virtual uint32_t           getRxFreq(       size_t   index                       ) const {
    return pMemory->channel[ index ].freq.rxHz();
  }
  virtual bool               setRxFreq(       size_t   index, uint32_t     value   ) {
    return pMemory->channel[ index ].freq.rxSet( value );
  }

  virtual bits_t             getSplit(        size_t   index                       ) const {
    return pMemory->channel[ index ].direction;
  }
  virtual bool               setSplit(        size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].direction = value;
    return true;
  }

  virtual uint32_t           getTxOffset(     size_t   index                       ) const {
    return pMemory->channel[ index ].freq.txHz();
  }
  virtual bool               setTxOffset(     size_t   index, uint32_t     value   ) {
    return pMemory->channel[ index ].freq.txSet( value );
  }

  virtual bits_t             getRxStep(       size_t   index                       ) const {
    return pMemory->channel[ index ].tuneStep;
  }
  virtual bool               setRxStep(       size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].tuneStep = value;
    return true;
  }
  virtual double const     * getTuneSteps(    size_t * count                       ) const {
    *count = COUNT_OF( tuneSteps );
    return tuneSteps;
  }

  virtual bits_t             getModulation(   size_t index                         ) const {
    return pMemory->channel[ index ].modulation;
  }
  virtual bool               setModulation(   size_t index, bits_t          value  ) {
    pMemory->channel[ index ].modulation = value;
    return true;
  }
  virtual char const * const * getModulations( size_t * count               ) const {
    *count = COUNT_OF( ::modulations );
    return ::modulations;
  }

  virtual char const * getName(         size_t   index, size_t     * pSize   ) const {
    *pSize = sizeof pMemory->channel[ 0 ].name;
    return pMemory->channel[ index ].name;
  }
  virtual bool               setName(         size_t   index, char const * pValue  ) {
    strpad( pMemory->channel[ index ].name, sizeof pMemory->channel[ 0 ].name, pValue, ' ' );
    return true;
  }

  virtual bits_t             getFmSquelch(    size_t   index                       ) const {
    return pMemory->channel[ index ].fmSquelch;
  }
  virtual bool               setFmSquelch(    size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].fmSquelch = value;
    return true;
  }
  virtual char const * const * getFmSquelches(  size_t * count                       ) const {
    *count = COUNT_OF( fmSquelches );
    return fmSquelches;
  }

  virtual bits_t             getCtcssEncode(  size_t   index                       ) const {
    return pMemory->channel[ index ].ctcssEncode;
  }
  virtual bool               setCtcssEncode(  size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].ctcssEncode = value;
    return true;
  }

  virtual bits_t             getCtcssDecode(  size_t   index                       ) const {
    return pMemory->channel[ index ].ctcssDecodeHi << 4 | pMemory->channel[ index ].ctcssDecodeLo;
  }
  virtual bool               setCtcssDecode(  size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].ctcssDecodeLo = value;
    pMemory->channel[ index ].ctcssDecodeLo = value >> 4;
    return true;
  }

  virtual bits_t             getDcsCode(      size_t   index                       ) const {
    return pMemory->channel[ index ].dcsCode;
  }
  virtual bool               setDcsCode(      size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dcsCode = value;
    return true;
  }

  virtual bits_t             getDcsReverse(   size_t   index                       ) const {
    return pMemory->channel[ index ].dcsReverse;
  }
  virtual bool               setDcsReverse(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dcsReverse = value;
    return true;
  }

  virtual bits_t             getDvSquelch(    size_t   index                       ) const {
    return pMemory->channel[ index ].dvSquelch;
  }
  virtual bool               setDvSquelch(    size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dvSquelch = value;
    return true;
  }

  virtual bits_t             getDvCsqlCode(   size_t   index                       ) const {
    return pMemory->channel[ index ].dvCsqlCode;
  }
  virtual bool               setDvCsqlCode(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dvCsqlCode = value;
    return true;
  }

  virtual Routing            getYourCall(     size_t   index                       ) const {
    char call[ 9 ];
    pMemory->channel[ index ].yourCall.unpack( call );
    return call;
  }
  virtual bool               setYourCall(     size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].yourCall = pValue;
    return true;
  }

  virtual Routing            getRpt1Call(     size_t   index                       ) const {
    char call[ 9 ];
    pMemory->channel[ index ].rpt1Call.unpack( call );
    return call;
  }
  virtual bool               setRpt1Call(     size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].rpt1Call = pValue;
    return true;
  }

  virtual Routing            getRpt2Call(     size_t   index                       ) const {
    char call[ 9 ];
    pMemory->channel[ index ].rpt2Call.unpack( call );
    return call;
  }
  virtual bool               setRpt2Call(     size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].rpt2Call = pValue;
    return true;
  }

  virtual bits_t             getBankGroup(    size_t   index                       ) const {
    return pMemory->bankMap[ index ].group;
  }
  virtual bool               setBankGroup(    size_t   index, bits_t       value   )       {
    pMemory->bankMap[ index ].group = value < (int)Id8X0Memory::BANKS ? value : (int)Id8X0Memory::BANKS;
    return true;
  }

  virtual bits_t             getBankChannel(  size_t   index                       ) const {
    return pMemory->bankMap[ index ].index;
  }
  virtual bool               setBankChannel(  size_t   index, bits_t       value   )       {
    pMemory->bankMap[ index ].index = value;
    return true;
  }

 public:
  Id8X0( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Dstar( pHeader, pData, size ), pMemory( (Id8X0Memory *)Dstar::pData ) {
    assert( sizeof( Id8X0Channel ) == 41 );
    assert( sizeof( Id8X0Memory  ) == size );
  }

  virtual ~Id8X0( void ) {}
};

class Id880 : public Id8X0 {
  Id880(             void              );  // Intentionally not implemented
  Id880(             Id880 const & rhs );  // Intentionally not implemented
  Id880 & operator=( Id880 const & rhs );  // Intentionally not implemented

  virtual char const         * getComment(                      char       * pExport ) const {
    _getComment( pExport );
    return "Icom ID-880H";
  }
  virtual char const         * setComment(                      char const * pExport )       {
    _setComment( pExport );
    return "Icom ID-880H";
  }
public:
  Id880( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Id8X0( pHeader, pData, size ) {
  }
  virtual ~Id880( void ) {}
};

class Ic80d : public Id8X0 {
  Ic80d(             void              );  // Intentionally not implemented
  Ic80d(             Ic80d const & rhs );  // Intentionally not implemented
  Ic80d & operator=( Ic80d const & rhs );  // Intentionally not implemented

  virtual char const         * getComment(                      char       * pExport ) const {
    _getComment( pExport );
    return "Icom IC-80AD";
  }
  virtual char const         * setComment(                      char const * pExport )       {
    _setComment( pExport );
    return "Icom IC-80AD";
  }
public:
  Ic80d( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Id8X0( pHeader, pData, size ) {
  }
  virtual ~Ic80d( void ) {}
};

Dstar * newId880( char    const * pHeader,
                uint8_t const * pData,
                size_t          size ) {
  return size == 0xF600  &&  memcmp( pHeader, "31670001", 8 ) == 0
         ? new Id880( pHeader, pData, size ) : 0;
}

Dstar * newIc80d( char    const * pHeader,
                uint8_t const * pData,
                size_t          size ) {
  return size == 0xF600  &&  memcmp( pHeader, "31550001", 8 ) == 0
         ? new Ic80d( pHeader, pData, size ) : 0;
}
