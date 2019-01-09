//==================================================================================================
//  IdDr.cpp is the Icom ID-31A/ID-51A/ID-5100 radio class file for Radio2csv.
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

static char const * const modulations[ 8 ] = { "FM", "FM-N", "?2?", "AM", "?4?", "DV", "?6?", "?7?" };

struct IdDrChannel : public _IdDrChannel< 16 >{};  // big-endian target

struct IdDrRepeater {     // big-endian target
  FrequencySetBE_18      freq;
  uint8_t HI2LOW_ORDER(  gpsExact  : 1, : 7 );
  uint8_t HI2LOW_ORDER(  direction : 2, : 6 );
  uint8_t HI2LOW_ORDER(  timeZone  : 7, : 1 );
  DstarCall              rpt1Call;
  char                   locality[ 16 ];
  char                   country[   8 ];
  char                   gpsStuff[ 10 ];
};

class IdDr : public Dstar {
  IdDr(             void             );  // Intentionally not implemented
  IdDr(             IdDr const & rhs );  // Intentionally not implemented
  IdDr & operator=( IdDr const & rhs );  // Intentionally not implemented

 protected:
  IdDrChannel       * const pChannel;

  virtual void                 setValid(        size_t   index, bool         value   ) {
    memset( & pChannel[ index ], value ? 0 : -1, sizeof pChannel[ 0 ] );
    pChannel[ index ].unknown1 = 0xE4;
    setName(     index, "" );
    setSkipMode( index, 0 );
    setIgnore(   index, !value );
  }

  virtual uint32_t             getRxFreq(       size_t   index                       ) const {
    return pChannel[ index ].freq.rxHz();
  }
  virtual bool                 setRxFreq(       size_t   index, uint32_t     value   ) {
    return pChannel[ index ].freq.rxSet( value );
  }

  virtual bits_t               getSplit(        size_t   index                       ) const {
    return pChannel[ index ].direction;
  }
  virtual bool                 setSplit(        size_t   index, bits_t       value   ) {
    pChannel[ index ].direction = value;
    return true;
  }

  virtual uint32_t             getTxOffset(     size_t   index                       ) const {
    return pChannel[ index ].freq.txHz();
  }
  virtual bool                 setTxOffset(     size_t   index, uint32_t     value   ) {
    return pChannel[ index ].freq.txSet( value );
  }

  virtual bits_t               getRxStep(       size_t   index                       ) const {
    return pChannel[ index ].tuneStep;
  }
  virtual bool                 setRxStep(       size_t   index, bits_t       value   ) {
    pChannel[ index ].tuneStep = value;
    return true;
  }
  virtual double const       * getTuneSteps(    size_t * count                       ) const {
    *count = COUNT_OF( tuneSteps );
    return tuneSteps;
  }

  virtual char const * const * getModulations(  size_t * count                       ) const {
    *count = COUNT_OF( ::modulations );
    return ::modulations;
  }

  virtual char const         * getName(         size_t   index, size_t     * pSize   ) const {
    *pSize = sizeof pChannel[ 0 ].name;
    return pChannel[ index ].name;
  }
  virtual bool                 setName(         size_t   index, char const * pValue  ) {
    strpad( pChannel[ index ].name, sizeof pChannel[ 0 ].name, pValue, ' ' );
    return true;
  }

  virtual bits_t               getFmSquelch(    size_t   index                       ) const {
    return pChannel[ index ].fmSquelch;
  }
  virtual bool                 setFmSquelch(    size_t   index, bits_t       value   ) {
    pChannel[ index ].fmSquelch = value;
    return true;
  }
  virtual char const * const * getFmSquelches(  size_t * count                       ) const {
    *count = COUNT_OF( fmSquelches ) - 4;
    return fmSquelches;
  }

  virtual bits_t               getCtcssEncode(  size_t   index                       ) const {
    return pChannel[ index ].ctcssEncode;
  }
  virtual bool                 setCtcssEncode(  size_t   index, bits_t       value   ) {
    pChannel[ index ].ctcssEncode = value;
    return true;
  }

  virtual bits_t               getCtcssDecode(  size_t   index                       ) const {
    return pChannel[ index ].ctcssDecodeHi << 4 | pChannel[ index ].ctcssDecodeLo;
  }
  virtual bool                 setCtcssDecode(  size_t   index, bits_t       value   ) {
    pChannel[ index ].ctcssDecodeLo = value;
    pChannel[ index ].ctcssDecodeHi = value >> 4;
    return true;
  }

  virtual bits_t               getDcsCode(      size_t   index                       ) const {
    return pChannel[ index ].dcsCode;
  }
  virtual bool                 setDcsCode(      size_t   index, bits_t       value   ) {
    pChannel[ index ].dcsCode = value;
    return true;
  }

  virtual bits_t               getDcsReverse(   size_t   index                       ) const {
    return pChannel[ index ].dcsReverse;
  }
  virtual bool                 setDcsReverse(   size_t   index, bits_t       value   ) {
    pChannel[ index ].dcsReverse = value;
    return true;
  }

  virtual bits_t               getDvSquelch(    size_t   index                       ) const {
    return pChannel[ index ].dvSquelch;
  }
  virtual bool                 setDvSquelch(    size_t   index, bits_t       value   ) {
    pChannel[ index ].dvSquelch = value;
    return true;
  }

  virtual bits_t               getDvCsqlCode(   size_t   index                       ) const {
    return pChannel[ index ].dvCsqlCode;
  }
  virtual bool                 setDvCsqlCode(   size_t   index, bits_t       value   ) {
    pChannel[ index ].dvCsqlCode = value;
    return true;
  }

  virtual Routing              getYourCall(     size_t   index                       ) const {
    char call[ 9 ];
    pChannel[ index ].yourCall.unpack( call );
    return call;
  }
  virtual bool                 setYourCall(     size_t   index, char const * pValue  ) {
    pChannel[ index ].yourCall = pValue;
    return true;
  }

  virtual Routing              getRpt1Call(     size_t   index                       ) const {
    char call[ 9 ];
    pChannel[ index ].rpt1Call.unpack( call );
    return call;
  }
  virtual bool                 setRpt1Call(     size_t   index, char const * pValue  ) {
    pChannel[ index ].rpt1Call = pValue;
    return true;
  }

  virtual Routing              getRpt2Call(     size_t   index                       ) const {
    char call[ 9 ];
    pChannel[ index ].rpt2Call.unpack( call );
    return call;
  }
  virtual bool                 setRpt2Call(     size_t   index, char const * pValue  ) {
    pChannel[ index ].rpt2Call = pValue;
    return true;
  }

  IdDr( char    const * pHeader,
        uint8_t const * pData,
        size_t          size )
      : Dstar( pHeader, pData, size ), pChannel( (IdDrChannel *)Dstar::pData ) {
    assert( sizeof( IdDrChannel  ) == 49 );
    assert( sizeof( IdDrRepeater ) == 49 );
  }
  virtual ~IdDr( void ) {}
};

// ==============================================================================

struct Id31Memory {
  enum {
    CHANNELS   = 500,
    URCALLS    = 200,
    EDGES      =  25,
    BANKS      =  26,
    SCAN_LISTS =  10,
    SCANS      =  25,
    REPEATERS  = 700,
    GROUPS     =  20
  };

  IdDrChannel  channel[        CHANNELS ];          // 0x00000 + 0x5FB4
  struct {
    IdDrChannel  lower,
                 upper;
  }            scanEdge[                  EDGES ];  // 0x05FB4 + 0x0992
  IdDrChannel  call[      2 ];                      // 0x06946 + 0x0062
  uint8_t      fill1[    24 ];                      // 0x069A8
  uint8_t      ignoreChannel[ (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 2 ];  // 0x069C0 + 0x0046
  uint8_t      skipChannel[   (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x06A06 + 0x0045
  uint8_t      skippChannel[  (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x06A4B + 0x0045
  uint8_t      fill2[  3*16 ];                                              // 0x06A90
  BankMap      bankMap[        CHANNELS + EDGES * 2 ];                      // 0x06AC0 + 0x044C
  uint8_t      fill3[    52 ];                                              // 0x06F0C
  char         comment[  16 ];                      // 0x06F40 + 0x0010
  char         bankName[       BANKS ][ 16 ];       // 0x06F50 + 0x01A0
  char         scanName[       SCANS ][ 16 ];       // 0x070F0 + 0x0190
  uint8_t      fill4[ 36*16 ];                      // 0x07280
  IdDrRepeater dstarRepeater[  REPEATERS ];         // 0x074C0 + 0x85FC
  DstarCall    rpCall[         REPEATERS ];         // 0x0FABC + 0x1324
  char         groupName[      GROUPS ][ 16 ];      // 0x10DE0 + 0x0140
  MyCall       myCall[    6 ];                      // 0x10F20 + 0x0048
  Routing      urCall[         URCALLS ];           // 0x10F68 + 0x0640
  uint8_t      fill5[ 16216 ];                      // 0x115A8
};                                                  // 0x15500

class Id31 : public IdDr {
  Id31Memory * pMemory;

  Id31(             void             );  // Intentionally not implemented
  Id31(             Id31 const & rhs );  // Intentionally not implemented
  Id31 & operator=( Id31 const & rhs );  // Intentionally not implemented

 protected:
  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
    return "Icom ID-31";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
    return "Icom ID-31";
  }
  virtual size_t getCount(        void                         ) const {
    return Id31Memory::CHANNELS;
  }

  virtual bool   getIgnore(       size_t   index               ) const {
    return getBit( pMemory->ignoreChannel, index );
  }
  virtual bool   setIgnore(       size_t   index, bool   value )       {
    return setBit( pMemory->ignoreChannel, index, value );
  }
  virtual bool   getSkip(         size_t   index               ) const {
    return getBit( pMemory->skipChannel,   index );
  }
  virtual bool   setSkip(         size_t   index, bool   value )       {
    return setBit( pMemory->skipChannel,   index, value );
  }
  virtual bool   getSkipp(        size_t   index               ) const {
    return getBit( pMemory->skippChannel,  index );
  }
  virtual bool   setSkipp(        size_t   index, bool   value )       {
    return setBit( pMemory->skippChannel,  index, value );
  }

  virtual bool   getValid(        size_t   index               ) const {
    return getIgnore( index ) == 0;
  }

  virtual bits_t getModulation(   size_t  index                ) const {
    return pChannel[ index ].modulation;
  }
  virtual bool   setModulation(   size_t   index, bits_t value )       {
    pChannel[ index ].modulation = value;
    return true;
  }

  virtual bits_t getBankGroup(    size_t   index               ) const {
    return pMemory->bankMap[ index ].group;
  }
  virtual bool   setBankGroup(    size_t   index, bits_t value )       {
    pMemory->bankMap[ index ].group = value;
    return true;
  }

  virtual bits_t getBankChannel(  size_t   index               ) const {
    return pMemory->bankMap[ index ].index;
  }
  virtual bool   setBankChannel(  size_t   index, bits_t value )       {
    pMemory->bankMap[ index ].index = value;
    return true;
  }

 public:
  Id31( char    const * pHeader,
        uint8_t const * pData,
        size_t          size )
      : IdDr( pHeader, pData, size ), pMemory( (Id31Memory *)Dstar::pData ) {
  }
  virtual ~Id31( void ) {}
};

Dstar * newId31( char    const * pHeader,
                 uint8_t const * pData,
                 size_t          size ) {
  return size == sizeof( Id31Memory )  &&  memcmp( pHeader, "33220001", 8 ) == 0
         ? new Id31( pHeader, pData, size ) : 0;
}

// ==============================================================================

struct Id51Memory {
  enum {
    CHANNELS   = 500,
    URCALLS    = 200,
    EDGES      =  25,
    BANKS      =  26,
    SCAN_LISTS =  10,
    SCANS      =  25,
    REPEATERS  = 750,
    STATIONS   = 500,
    GROUPS     =  25
  };

  IdDrChannel  channel[        CHANNELS ];          // 0x00000 + 0x5FB4
  struct {
    IdDrChannel  lower,
                 upper;
  }            scanEdge[                  EDGES ];  // 0x05FB4 + 0x0992
  IdDrChannel  call[        4 ];                    // 0x06946 + 0x00C4
  uint8_t      fill1[      54 ];                    // 0x06A0A
  uint8_t      ignoreChannel[ (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 2 ];  // 0x06A40 + 0x0046
  uint8_t      skipChannel[   (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x06A86 + 0x0045
  uint8_t      skippChannel[  (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x06ACB + 0x0045
  uint8_t      fill2[    3*16 ];                                            // 0x06B10
  BankMap      bankMap[        CHANNELS + EDGES * 2 ];                      // 0x06B40 + 0x044C
  uint8_t      fill3[      52 ];                    // 0x06F8C
  char         comment[    16 ];                    // 0x06FC0 + 0x0010
  char         bankName[       BANKS ][ 16 ];       // 0x06FD0 + 0x01A0
  char         scanName[       SCANS ][ 16 ];       // 0x07170 + 0x0190
  uint8_t      fill4[   36*16 ];                    // 0x07300
  struct {
    FrequencyBE< 32 > frequency;
    char              name[ 16 ];
  }            bcStation[      STATIONS ];          // 0x07540 + 0x2710
  uint8_t      fill5[    2663 ];                    // 0x09C50
  char         bcBankName[     BANKS ][ 16 ];       // 0x0A6B7 + 0x01A0
  uint8_t      fill6[     105 ];                    // 0x0A857
  IdDrRepeater dstarRepeater[  REPEATERS ];         // 0x0A8C0 + 0x8F8E
  DstarCall    rpCall[         REPEATERS ];         // 0x1384E + 0x1482
  char         groupName[      GROUPS ][ 16 ];      // 0x14CD0 + 0x0190
  MyCall       myCall[      6 ];                    // 0x14E60 + 0x0048
  Routing      urCall[         URCALLS ];           // 0x14EA8 + 0x0640
  uint8_t      fill8[   42584 ];                    // 0x154E8
};                                                  // 0x1FB40

class Id51x : public IdDr {
  Id51x(             void              );  // Intentionally not implemented
  Id51x(             Id51x const & rhs );  // Intentionally not implemented
  Id51x & operator=( Id51x const & rhs );  // Intentionally not implemented

 protected:
  Id51Memory * pMemory;

  virtual size_t getCount(        void                         ) const {
    return Id51Memory::CHANNELS;
  }

  virtual bool   getIgnore(       size_t   index               ) const {
    return getBit( pMemory->ignoreChannel, index );
  }
  virtual bool   setIgnore(       size_t   index, bool   value )       {
    return setBit( pMemory->ignoreChannel, index, value );
  }
  virtual bool   getSkip(         size_t   index               ) const {
    return getBit( pMemory->skipChannel,   index );
  }
  virtual bool   setSkip(         size_t   index, bool   value )       {
    return setBit( pMemory->skipChannel,   index, value );
  }
  virtual bool   getSkipp(        size_t   index               ) const {
    return getBit( pMemory->skippChannel,  index );
  }
  virtual bool   setSkipp(        size_t   index, bool   value )       {
    return setBit( pMemory->skippChannel,  index, value );
  }

  virtual bool   getValid(        size_t   index               ) const {
    return getIgnore( index ) == 0;
  }

  virtual bits_t getModulation(   size_t  index                ) const {
    return pChannel[ index ].modulation;
  }
  virtual bool   setModulation(   size_t   index, bits_t value )       {
    pChannel[ index ].modulation = value;
    return true;
  }

  virtual bits_t getBankGroup(    size_t   index               ) const {
    return pMemory->bankMap[ index ].group;
  }
  virtual bool   setBankGroup(    size_t   index, bits_t value )       {
    pMemory->bankMap[ index ].group = value;
    return true;
  }

  virtual bits_t getBankChannel(  size_t   index               ) const {
    return pMemory->bankMap[ index ].index;
  }
  virtual bool   setBankChannel(  size_t   index, bits_t value )       {
    pMemory->bankMap[ index ].index = value;
    return true;
  }

 public:
  Id51x( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : IdDr( pHeader, pData, size ), pMemory( (Id51Memory *)Dstar::pData ) {
    assert( sizeof( Id51Memory ) == size );
  }
  virtual ~Id51x( void ) {}
};

class Id51 : public Id51x {
  Id51(             void             );  // Intentionally not implemented
  Id51(             Id51 const & rhs );  // Intentionally not implemented
  Id51 & operator=( Id51 const & rhs );  // Intentionally not implemented
  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
    return "Icom ID-51";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
    return "Icom ID-51";
  }

 public:
  Id51( char    const * pHeader,
        uint8_t const * pData,
        size_t          size )
      : Id51x( pHeader, pData, size ) {
  }
  virtual ~Id51( void ) {}
};

class Id51p : public Id51x {
  Id51p(             void              );  // Intentionally not implemented
  Id51p(             Id51p const & rhs );  // Intentionally not implemented
  Id51p & operator=( Id51p const & rhs );  // Intentionally not implemented
  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
    return "Icom ID-51+";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
    return "Icom ID-51+";
  }

 public:
  Id51p( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Id51x( pHeader, pData, size ) {
  }
  virtual ~Id51p( void ) {}
};

class Id51p2 : public Id51x {
  Id51p2(             void               );  // Intentionally not implemented
  Id51p2(             Id51p2 const & rhs );  // Intentionally not implemented
  Id51p2 & operator=( Id51p2 const & rhs );  // Intentionally not implemented
  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
    return "Icom ID-51++";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
    return "Icom ID-51++";
  }

protected:
  virtual char const * const * getFmSquelches( size_t * count               ) const {
    *count = COUNT_OF( fmSquelches );
    return fmSquelches;
  }

 public:
  Id51p2( char    const * pHeader,
          uint8_t const * pData,
          size_t          size )
      : Id51x( pHeader, pData, size ) {
  }
  virtual ~Id51p2( void ) {}
};

Dstar * newId51( char    const * pHeader,
                 uint8_t const * pData,
                 size_t          size ) {
  return size == 0x1FB40  &&  memcmp( pHeader, "33900001", 8 ) == 0
         ? new Id51( pHeader, pData, size ) : 0;
}

Dstar * newId51p( char    const * pHeader,
                  uint8_t const * pData,
                  size_t          size ) {
  return size == 0x1FB40  &&  memcmp( pHeader, "33900002", 8 ) == 0
         ? new Id51p( pHeader, pData, size ) : 0;
}

Dstar * newId51p2( char    const * pHeader,
                   uint8_t const * pData,
                   size_t          size ) {
  return size == 0x1FB40  &&  memcmp( pHeader, "33900003", 8 ) == 0
         ? new Id51p2( pHeader, pData, size ) : 0;
}

// ==============================================================================

struct Id5100Memory {
  enum {
    CHANNELS  = 1000,
    URCALLS   =  300,
    EDGES     =   25,
    BANKS     =   26,
    SCAN_LISTS =  10,
    SCANS      =  25,
    REPEATERS = 1500,
    GROUPS    =   50
  };

  IdDrChannel channel[        CHANNELS ];          // 0x000000 + 0x0BF68
  IdDrChannel call[        4 ];                    // 0x00BF68 + 0x000C4
  uint8_t     fill1[      20 ];                    // 0x00C02C + 0x00014
  uint8_t     ignoreChannel[ (CHANNELS - 1) / CHAR_BIT + 2 ];  // 0x00C040 + 0x0007E
  uint8_t     skipChannel[   (CHANNELS - 1) / CHAR_BIT + 1 ];  // 0x00C0BE + 0x0007D
  uint8_t     skippChannel[  (CHANNELS - 1) / CHAR_BIT + 1 ];  // 0x00C13B + 0x0007D
  uint8_t     fill2[       8 ];                    // 0x00C1B8 + 0x00008
  BankMap     bankMap[        CHANNELS ];          // 0x00C1C0 + 0x007D0
  uint8_t     fill3[    3*16 ];                    // 0x00C990 + 0x00030
  char        comment[    16 ];                    // 0x00C9C0 + 0x00010
  char        bankName[       BANKS ][ 16 ];       // 0x00C9D0 + 0x001A0
  uint8_t     fill4[   37*16 ];                    // 0x00CB70 + 0x0024D
  struct {
    uint32_t  lower,
              upper;
    char      name[ 16 ];
  }           scanEdge[       EDGES ];             // 0x00CDC0 + 0x00258
  uint8_t     fill5[      40 ];                    // 0x00D018 + 0x00028
  uint8_t     dstarRepeater[  REPEATERS * 57 ];    // 0x00D040 + 0x14DFC
  char        groupName[      GROUPS ][ 16 ];      // 0x021E3C + 0x00320
  MyCall      myCall[    6 ];                      // 0x02215C + 0x00048
  Routing     urCall[         URCALLS ];           // 0x0221A4 + 0x00960
  char        urCallName[     URCALLS ][ 16 ];     // 0x022B04 + 0x012C0
  uint8_t     fill9[   26028 ];                    // 0x023DC4
  char        ident[      16 ];                    // 0x02A370 = "IcomCloneFormat3"
};                                                 // 0x02A380

class Id5100 : public IdDr {
  Id5100Memory * pMemory;

  Id5100(             void               );  // Intentionally not implemented
  Id5100(             Id5100 const & rhs );  // Intentionally not implemented
  Id5100 & operator=( Id5100 const & rhs );  // Intentionally not implemented
  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
    return "Icom ID-5100";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
    return "Icom ID-5100";
  }

 protected:
  virtual size_t               getCount(       void                         ) const {
    return Id5100Memory::CHANNELS;
  }

  virtual bool                 getIgnore(      size_t   index               ) const {
    return getBit( pMemory->ignoreChannel, index );
  }
  virtual bool                 setIgnore(      size_t   index, bool   value )       {
    return setBit( pMemory->ignoreChannel, index, value );
  }
  virtual bool                 getSkip(        size_t   index               ) const {
    return getBit( pMemory->skipChannel,   index );
  }
  virtual bool                 setSkip(        size_t   index, bool   value )       {
    return setBit( pMemory->skipChannel,   index, value );
  }
  virtual bool                 getSkipp(       size_t   index               ) const {
    return getBit( pMemory->skippChannel,  index );
  }
  virtual bool                 setSkipp(       size_t   index, bool   value )       {
    return setBit( pMemory->skippChannel,  index, value );
  }

  virtual bool                 getValid(       size_t   index               ) const {
    return getIgnore( index ) == 0;
  }

  virtual bits_t               getModulation(  size_t   index               ) const {
    return pChannel[ index ].modulation;
  }
  virtual bool                 setModulation(  size_t   index, bits_t value )       {
    pChannel[ index ].modulation = value;
    return true;
  }

  virtual char const * const * getFmSquelches( size_t * count               ) const {
    *count = COUNT_OF( fmSquelches );
    return fmSquelches;
  }

  virtual bits_t               getBankGroup(   size_t   index               ) const {
    return pMemory->bankMap[ index ].group;
  }
  virtual bool                 setBankGroup(   size_t   index, bits_t value )       {
    pMemory->bankMap[ index ].group = value;
    return true;
  }

  virtual bits_t               getBankChannel( size_t   index               ) const {
    return pMemory->bankMap[ index ].index;
  }
  virtual bool                 setBankChannel( size_t   index, bits_t value )       {
    pMemory->bankMap[ index ].index = value;
    return true;
  }

 public:
  Id5100( char    const * pHeader,
          uint8_t const * pData,
          size_t          size )
      : IdDr( pHeader, pData, size ), pMemory( (Id5100Memory *)Dstar::pData ) {
    assert( sizeof( Id5100Memory ) == size );
  }
  virtual ~Id5100( void ) {}
};

Dstar * newId5100( char    const * pHeader,
                   uint8_t const * pData,
                   size_t          size ) {
  return size == 0x2A380  &&  memcmp( pHeader, "34840001", 8 ) == 0
         ? new Id5100( pHeader, pData, size ) : 0;
}
