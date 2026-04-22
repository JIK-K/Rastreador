cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("Rastreador")

# 인클루드 경로
include_directories(include resources)
add_compile_options(/utf-8)
# 전처리기 정의
add_definitions(
    -D_WIN32_WINNT=0x0602
    -DUNICODE
    -D_UNICODE
)

# 소스 파일
add_executable(Rastreador
    "src/Rastreador.cpp"
    "src/collector/SystemMonitor.cpp"
    "src/collector/ProcessMonitor.cpp"
    "src/collector/ETWMonitor.cpp"
    "src/analyzer/BottleneckAnalyzer.cpp"
    "src/display/OverlayDisplay.cpp"
    "src/core/Monitor.cpp"
    "src/tray/TrayIcon.cpp"
    "resources/resource.rc"      
)

# C++ 표준
set_property(TARGET Rastreador PROPERTY CXX_STANDARD 20)

# 컴파일 옵션 (Debug / Release)
target_compile_options(Rastreador PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /fsanitize=address>
    $<$<CONFIG:Release>:/O2>
)

# 링크 라이브러리
target_link_libraries(Rastreador PRIVATE
    pdh
    iphlpapi
    psapi
    ws2_32
    tdh
    advapi32
    gdiplus
)