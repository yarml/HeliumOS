ifeq ($(origin CC),default)
	undefine CC
endif
ifeq ($(origin CXX),default)
	undefine CXX
endif
ifeq ($(origin AS),default)
	undefine AS
endif
ifeq ($(origin LD),default)
	undefine LD
endif
ifeq ($(origin RM),default)
	undefine RM
endif
ifeq ($(origin MKDIR),default)
	undefine MKDIR
endif
ifeq ($(origin CP),default)
	undefine CP
endif
ifeq ($(origin M4),default)
	undefine M4
endif
ifeq ($(origin CD),default)
	undefine CD
endif