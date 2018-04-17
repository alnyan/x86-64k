#pragma once
#include <algo/option.hpp>

using error = int;

class result {
public:
    result(error e): m_error{e == 0 ? option<error>::none() : option<error>::some(e)} {
    }

    operator bool() const {
        return !m_error;
    }

    error operator *() const {
        return m_error ? *m_error : 0;
    }

private:
    const option<error> m_error;
};
