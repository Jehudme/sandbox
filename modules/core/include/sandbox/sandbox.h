#pragma once

#ifdef SWIG
%include <sandbox/services/application_service.h>
%include <sandbox/services/configuration_service.h>
%include <sandbox/services/filesystem_service.h>
%include <sandbox/services/logs_service.h>
%include <sandbox/services/runtime_service.h>
#else
#include <sandbox/services/application_service.h>
#include <sandbox/services/configuration_service.h>
#include <sandbox/services/filesystem_service.h>
#include <sandbox/services/logs_service.h>
#include <sandbox/services/runtime_service.h>
#endif
