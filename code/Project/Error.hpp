#pragma once

#include <unordered_map>
#include "cmsis_os2.h"

inline const char* _ERROR_CODE_NOT_EXISTS = "Wrong error code";

class Error {
private:

    using errLUT_t = std::unordered_map<uint32_t, const char*>;
    static inline errLUT_t _errorLookupTable{};

    uint32_t _value;
    const char* _msg; 

public:

    /**
     * @brief Construct a new Error object by specifying
     * an error code and a descriptive message for it. 
     * 
     * It is advised against mixing manual and automatic error
     * code assignation. This constructor should be avoided
     * when using the second one with a single parameter. 
     * Otherwise an error code overlap could happen that can lead
     * to loss of error type information.  
     * 
     * @param errorCode Code representing a specific type of error.
     * @param errorMesage Descriptive message for an error type.
     */
    explicit Error(const uint32_t errorCode, const char* errorMesage) 
    : _value{errorCode}, _msg{errorMesage} {
        _errorLookupTable[_value] = _msg;
    }

    /**
     * @brief Construct a new Error object by only
     * specifying an error message and a unique error code is assigned
     * to it automatically. Error codes are set by number of elements
     * inside the error lookup table. 
     * 
     * @param errorMesage Descriptive message for an error type.
     */
    explicit Error(const char* errorMesage): _msg{errorMesage} {
        _value = (static_cast<uint32_t>(_errorLookupTable.size()))+1;
        _errorLookupTable[_value] = _msg;
    }
    
    // Cast Error to integer value
    operator uint32_t() const { return _value; }

    const char* getErrorMessage() const {
        return _msg;
    }    

    static const char* getErrorMessage(const uint32_t _errorCode) {
        errLUT_t::iterator it = _errorLookupTable.find(_errorCode);
        if(it == _errorLookupTable.end()) {
            return _ERROR_CODE_NOT_EXISTS;
        }
        return it->second;
    }
};