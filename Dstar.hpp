//==================================================================================================
//  Dstar.hpp is the Radio class file for Radio2csv.
//
//  Author:  Copyright (c) 2017-2017 by Dean K. Gibson.
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

#include "Radio.hpp"

struct MyCall {
  char call[ 8 ];
  char tag[  4 ];
};

struct BankMap {
  uint8_t HI2LOW_ORDER(       : 3,
                        group : 5 );
  uint8_t index;
};

typedef PackedString< 8, 7, 0 > DstarCall;

struct Routing {
  static Routing const cqcqcq;
  static Routing const direct;
  static Routing const notUse;
  char callsign[ 8 ];
  Routing( void ) {}
  Routing( char const * pCallsign ) {
    *this = pCallsign;
  }
  Routing & operator=( Routing const & rhs ) {
    memcpy( callsign, rhs.callsign, sizeof callsign );
    return *this;
  }
  Routing & operator=( char const * pCallsign ) {
    strpad( callsign, sizeof callsign, pCallsign, ' ' );
    return *this;
  }
  bool operator==( Routing const rhs ) const {
     return memcmp( callsign, rhs.callsign, sizeof callsign ) == 0;
  }
  bits_t lookup( Routing const * pDefault, Routing * pTable, size_t count ) const {
    size_t index;
    int    avail = -1;

    if ( *this == *pDefault ) {
      return 0;
    }
    for ( index = 0;  index < count;  index ++ ) {
      if ( *this == pTable[ index ] ) {
        return index + 1;
      }
      if ( avail < 0  &&  notUse == pTable[ index ] ) {
        avail = (int)index;
      }
    }
    if ( avail >= 0 ) {
      pTable[ avail ] = *this;
    }
    return avail + 1;
  }
  bool escape( char * pExport ) const {
    return ::escape( pExport, callsign, sizeof callsign );
  }
};

template< int SIZE >
struct _IdDrChannel {     // big-endian target
  FrequencySetBE_18       freq;
  uint8_t HI2LOW_ORDER(   ctcssEncode   : 6,
                          ctcssDecodeHi : 2 );
  uint8_t HI2LOW_ORDER(   ctcssDecodeLo : 4,
          HI2LOW_ORDER(                 : 1,
                          modulation    : 3 ));
  uint8_t HI2LOW_ORDER(                 : 1,
                          dcsCode       : 7 );
  uint8_t HI2LOW_ORDER(   tuneStep      : 4,
          HI2LOW_ORDER(   dvSquelch     : 2,
                                        : 2 ));
  uint8_t                 unknown1;          // 0xE4
  uint8_t HI2LOW_ORDER(   fmSquelch     : 4,
          HI2LOW_ORDER(   direction     : 2,
                          dcsReverse    : 2 ));
  char                    name[ SIZE ];
  uint8_t                 dvCsqlCode;
  DstarCall yourCall;
  DstarCall rpt1Call;
  DstarCall rpt2Call;
};

class Dstar : public Radio {
 private:
  Dstar(             void              );  // Intentionally not implemented
  Dstar(             Dstar const & rhs );  // Intentionally not implemented
  Dstar & operator=( Dstar const & rhs );  // Intentionally not implemented

 protected:
  static char const * const dvSquelches[  3 ];
  static char const * const fmSquelches[ 12 ];
  static double       const tuneSteps[   14 ];
  static CsvField     const csvFields[];

  virtual CsvField const * csvHeader( void ) const {
    return csvFields;
  }

  virtual bool                 getIgnore(        size_t   index                             ) const = 0;
  virtual bool                 setIgnore(        size_t   index, bool               value   )       = 0;
  virtual bool                 getSkip(          size_t   index                             ) const = 0;
  virtual bool                 setSkip(          size_t   index, bool               value   )       = 0;
  virtual bool                 getSkipp(         size_t   index                             ) const = 0;
  virtual bool                 setSkipp(         size_t   index, bool               value   )       = 0;

  virtual bits_t               getSkipMode(      size_t   index                             ) const {
    return getSkipp( index ) ? 2 : getSkip(  index ) ? 1 : 0;
  }
  virtual bool                 setSkipMode(      size_t   index, bits_t             value   ) {
    setSkipp( index, false );
    setSkip(  index, false );
    switch ( value ) {
      case 0:
        return true;
      case 2:
        setSkipp( index, true );
        // fall-thru
      case 1:
        setSkip(  index, true );
        return true;
    }
    return false;
  }
  virtual bool                _getSkipMode(      size_t   index, char             * pExport ) const {
    size_t skipMode = getSkipMode( index );
    if ( skipMode >= COUNT_OF( skipModes ) ) {
      sprintf( pExport, "%d", (int)skipMode );
      return false;
    }
    strcpy( pExport, skipModes[ skipMode ] );
    return true;
  }
  virtual bool                _setSkipMode(      size_t   index, char const       * pImport ) {
    int result = search( pImport, skipModes, COUNT_OF( skipModes ) );
    if ( result < 0 ) {
      return false;
    }
    return setSkipMode( index, result );
  }

  virtual bits_t               getDvSquelch(     size_t   index                             ) const = 0;
  virtual bool                 setDvSquelch(     size_t   index, bits_t             value   )       = 0;
  virtual char const * const * getDvSquelches(   size_t * count                             ) const {
    *count = COUNT_OF( dvSquelches );
    return dvSquelches;
  }
  virtual bool                _getDvSquelch(     size_t   index, char             * pExport ) const {
    size_t               count;
    char const * const * dvSquelches = getDvSquelches( & count );
    size_t               value       = getDvSquelch( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    strcpy( pExport, dvSquelches[ value ] );
    return true;
  }
  virtual bool                _setDvSquelch(     size_t   index, char const       * pImport )       {
    size_t               count;
    char const * const * dvSquelches = getDvSquelches( & count );
    int                  result      = search( pImport, dvSquelches, count );
    if ( result < 0 ) {
      return false;
    }
    return setDvSquelch( index, result );
  }

  virtual bits_t               getDvCsqlCode(    size_t   index                       ) const = 0;
  virtual bool                 setDvCsqlCode(    size_t   index, bits_t       value   )       = 0;
  virtual bool                _getDvCsqlCode(    size_t   index, char       * pExport ) const {
    size_t value = getDvCsqlCode( index );
    sprintf( pExport, "%02d", (int)value );
    return value < 100;
  }
  virtual bool                _setDvCsqlCode(    size_t   index, char const * pImport ) {
    char * pTemp;
    int result = strtoul( pImport, & pTemp, 10 );
    if ( *pTemp != 0  ||  result < 0  ||  result > 99 ) {
      return false;
    }
    return setDvCsqlCode( index, result );
  }

  virtual Routing              getYourCall(      size_t   index                       ) const = 0;
  virtual bool                 setYourCall(      size_t   index, char const * pValue  )       = 0;
  virtual bool                _getYourCall(      size_t   index, char       * pExport ) const {
    return getYourCall( index ).escape( pExport );
  }
  virtual bool                _setYourCall(      size_t   index, char const * pImport )       {
    return setYourCall( index, pImport );
  }

  virtual Routing              getRpt1Call(      size_t   index                       ) const = 0;
  virtual bool                 setRpt1Call(      size_t   index, char const * pValue  )       = 0;
  virtual bool                _getRpt1Call(      size_t   index, char       * pExport ) const {
    return getRpt1Call( index ).escape( pExport );
  }
  virtual bool                _setRpt1Call(      size_t   index, char const * pImport )       {
    return setRpt1Call( index, pImport );
  }

  virtual Routing              getRpt2Call(      size_t   index                       ) const = 0;
  virtual bool                 setRpt2Call(      size_t   index, char const * pValue  )       = 0;
  virtual bool                _getRpt2Call(      size_t   index, char       * pExport ) const {
    return getRpt2Call( index ).escape( pExport );
  }
  virtual bool                _setRpt2Call(      size_t   index, char const * pImport )       {
    return setRpt2Call( index, pImport );
  }

  virtual bits_t               getBankGroup(     size_t   index                       ) const = 0;
  virtual bool                 setBankGroup(     size_t   index, bits_t       value   )       = 0;
  virtual bool                _getBankGroup(     size_t   index, char       * pExport ) const {
    size_t value = getBankGroup( index );
    sprintf( pExport, "%c", value > 'Z' - 'A' ? ' ' : (int)value + 'A' );
    return true;
  }
  virtual bool                _setBankGroup(     size_t   index, char const * pImport ) {
    int result = -1;
    if ( *pImport ) {
      result = toupper( *pImport ) - 'A';
      if ( result < 0  ||  result > 25 ) {
        return false;
      }
    }
    return setBankGroup( index, result );
  }

  virtual bits_t               getBankChannel(   size_t   index                       ) const = 0;
  virtual bool                 setBankChannel(   size_t   index, bits_t       value   )       = 0;
  virtual bool                _getBankChannel(   size_t   index, char       * pExport ) const {
    size_t value = getBankChannel( index );
    sprintf( pExport, "%02d", value > 99 ? 0 : (int)value );
    return true;
  }
  virtual bool                _setBankChannel(   size_t   index, char const * pImport ) {
    char * pTemp;
    int result = strtoul( pImport, & pTemp, 10 );
    if ( *pTemp != 0  ||  result < 0  ||  result > 99 ) {
      return false;
    }
    return setBankChannel( index, result );
  }

  Dstar( char const * pHeader, uint8_t const * pData, size_t size )
      : Radio( pHeader, pData, size ) {}

 public:
  virtual ~Dstar( void ) {}
};
