/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_STATUS_HPP
#define CHEFERD_STATUS_HPP

#include <string>

namespace cheferd {

/**
 * PStatus class.
 * This class contains the primitives of Status messages.
 * Currently, the PStatus class contains the following variables:
 * - state_: Current status (e.g., ok, notsupported, error, nostatus)
 */
class PStatus {

private:
    enum class StatusCode { ok = 0, notsupported = 1, error = 2, nostatus = -1 };

public:
    StatusCode state_;

    /**
     * PStatus default constructor.
     */
    PStatus ();

    /**
     * PStatus parameterized constructor.
     * @param code Status code.
     */
    PStatus (StatusCode code);

    /**
     * PStatus default destructor.
     */
    ~PStatus ();

    /**
     * OK: Returns OK status code.
     * @return OK status code.
     */
    static PStatus OK ();

    /**
     * NotSupported: Returns NotSupported status code.
     * @return NotSupported status code.
     */
    static PStatus NotSupported ();

    /**
     * Error: Returns Error status code.
     * @return Error status code.
     */
    static PStatus Error ();

    /**
     * isOk: Checks if status is OK.
     * @return True if status is OK, false otherwise.
     */
    bool isOk ();

    /**
     * isNotSupported: Checks if status is NotSupported.
     * @return True if status is NotSupported, false otherwise.
     */
    bool isNotSupported ();

    /**
     * isError: Checks if status is isError.
     * @return True if status is isError, false otherwise.
     */
    bool isError ();

    /**
     * toString: Converts PStatus into string format.
     * @return PStatus in string format.
     */
    std::string toString ();
};
} // namespace cheferd

#endif // CHEFERD_STATUS_HPP
