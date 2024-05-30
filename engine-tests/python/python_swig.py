# Copyright (c) 2022-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys, gc
import Ogre, swig_test

#
# C++ object get and manipulate
#

v1 = swig_test.getV1()
print(sys.getrefcount(v1)-1) # 1 is OK, see tests/pybind_return_value_policy.cpp

print(v1) # value test
swig_test.showV1() # value test from C++

v1.x = 99  # value update
swig_test.showV1() # value test from C++
print(v1) # value test

#
# temporary C++ object get and use
#

v2 = swig_test.getV2()
print(sys.getrefcount(v2)-1) # should be 1

print(v2) # value test
swig_test.showV2(v2) # value test from C++

v2.x = 99  # value update
swig_test.showV2(v2)  # value test from C++
print(v2) # value test

#
# python object get and use
#

v3 = Ogre.Vector2(10,21)
print(sys.getrefcount(v3)-1) # should be 1

print(v2) # value test
swig_test.showV2(v3) # value test from C++

v2.x = 99  # value update
swig_test.showV2(v2)  # value test from C++
print(v2) # value test

#
# inter-operating
#

print(v1+v2+v3)
swig_test.showV2(v1+v2+v3)
print(v3.angleTo(v1))
print(v3.angleTo(v2))

# deleting test
del v1
gc.collect()
swig_test.showV1() # value test from C++
