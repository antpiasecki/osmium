#pragma once

#include <string_view>

const std::string_view GLOBAL_STYLE = R"(
.h1, .h2, .h3, .h4, .h5, .h6 {
    margin: 0.67em 0;
    font-weight: bold;
}

.h1 { font-size: 2em; }
.h2 { font-size: 1.5em; }
.h3 { font-size: 1.17em; }
.h4 { font-size: 1em; }
.h5 { font-size: 0.83em;}
.h6 { font-size: 0.67em; }

.p {
  margin: 1em 0;
}
)";