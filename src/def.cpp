#include "def.h"

pCode make_code(String code, String file_name) {
    return pCode(new Code{pString(new String(code)), file_name});
}
