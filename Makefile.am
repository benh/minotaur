# Makefile for minotaur.

ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign

SUBDIRS = third_party . # Build 'third_party' and then '.'.

LIBPROCESS = third_party/libprocess-0.0.1

noinst_PROGRAMS = simulator
simulator_SOURCES = src/main.cpp
simulator_CPPFLAGS = -I$(srcdir)/$(LIBPROCESS)/include
simulator_LDADD = $(LIBPROCESS)/libprocess.la
