/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_STATUS_HPP
#define SHEPHERD_STATUS_HPP

#include <string>

namespace shepherd {

class PStatus {

private:
    enum class StatusCode { ok = 0, notsupported = 1, error = 2, nostatus = -1 };

public:
    StatusCode state_;

    PStatus ();
    PStatus (StatusCode code);
    ~PStatus ();

    static PStatus OK ();

    static PStatus NotSupported ();

    static PStatus Error ();

    bool isOk ();

    bool isNotSupported ();

    bool isError ();

    std::string toString ();
};
} // namespace shepherd

#endif // SHEPHERD_STATUS_HPP
