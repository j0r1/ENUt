EDM Network Utilities (ENUt)
============================

Introduction

ENUt stands for 'EDM Network Utilities', a library meant to make network
programming a bit easier.

License

The license which applies to this library is the LGPL. You can find the
full version in the file LICENSE.LGPL which is included in the library
archive. The short version is the following:

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  
    USA.

Installation
------------

The library depends on ErrUt, which can be found here:
http://research.edm.uhasselt.be/jori/errut/errut.html

Use the CMake build system to compile the library. In case extra
include directories or libraries are needed, you can use the 
'ADDITIONAL_' CMake variables to specify these. They will be stored in 
both the resulting CMake configuration file and the pkg-config file.

Homepage
--------

The library homepage can be found at the following location:
http://research.edm.uhasselt.be/enut/

