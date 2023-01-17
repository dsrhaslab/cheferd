/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_STATUS_HPP
#define CHEFERD_STATUS_HPP

#include <string>

namespace cheferd {

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
} // namespace cheferd

#endif // CHEFERD_STATUS_HPP
