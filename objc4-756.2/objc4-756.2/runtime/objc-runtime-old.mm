/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/***********************************************************************
* objc-runtime-old.m
* Support for old-ABI classes and images.
**********************************************************************/

/***********************************************************************
 * Class loading and connecting (GrP 2004-2-11)
 *
 * When images are loaded (during program startup or otherwise), the 
 * runtime needs to load classes and categories from the images, connect 
 * classes to superclasses and categories to parent classes, and call 
 * +load methods. 
 * 
 * The Objective-C runtime can cope with classes arriving in any order. 
 * That is, a class may be discovered by the runtime before some 
 * superclass is known. To handle out-of-order class loads, the 
 * runtime uses a "pending class" system. 
 * 
 * (Historical note)
 * Panther and earlier: many classes arrived out-of-order because of 
 *   the poorly-ordered callback from dyld. However, the runtime's 
 *   pending mechanism only handled "missing superclass" and not 
 *   "present superclass but missing higher class". See Radar #3225652. 
 * Tiger: The runtime's pending mechanism was augmented to handle 
 *   arbitrary missing classes. In addition, dyld was rewritten and 
 *   now sends the callbacks in strictly bottom-up link order. 
 *   The pending mechanism may now be needed only for rare and 
 *   hard to construct programs.
 * (End historical note)
 * 
 * A class when first seen in an image is considered "unconnected". 
 * It is stored in `unconnected_class_hash`. If all of the class's 
 * superclasses exist and are already "connected", then the new class 
 * can be connected to its superclasses and moved to `class_hash` for 
 * normal use. Otherwise, the class waits in `unconnected_class_hash` 
 * until the superclasses finish connecting.
 * 
 * A "connected" class is 
 * (1) in `class_hash`, 
 * (2) connected to its superclasses, 
 * (3) has no unconnected superclasses, 
 * (4) is otherwise initialized and ready for use, and 
 * (5) is eligible for +load if +load has not already been called. 
 * 
 * An "unconnected" class is 
 * (1) in `unconnected_class_hash`, 
 * (2) not connected to its superclasses, 
 * (3) has an immediate superclass which is either missing or unconnected, 
 * (4) is not ready for use, and 
 * (5) is not yet eligible for +load.
 * 
 * Image mapping is NOT CURRENTLY THREAD-SAFE with respect to just about 
 * anything. Image mapping IS RE-ENTRANT in several places: superclass 
 * lookup may cause ZeroLink to load another image, and +load calls may 
 * cause dyld to load another image.
 * 
 * Image mapping sequence:
 * 
 * Read all classes in all new images. 
 *   Add them all to unconnected_class_hash. 
 *   Note any +load implementations before categories are attached.
 *   Attach any pending categories.
 * Read all categories in all new images. 
 *   Attach categories whose parent class exists (connected or not), 
 *     and pend the rest.
 *   Mark them all eligible for +load (if implemented), even if the 
 *     parent class is missing.
 * Try to connect all classes in all new images. 
 *   If the superclass is missing, pend the class
 *   If the superclass is unconnected, try to recursively connect it
 *   If the superclass is connected:
 *     connect the class
 *     mark the class eligible for +load, if implemented
 *     fix up any pended classrefs referring to the class
 *     connect any pended subclasses of the class
 * Resolve selector refs and class refs in all new images.
 *   Class refs whose classes still do not exist are pended.
 * Fix up protocol objects in all new images.
 * Call +load for classes and categories.
 *   May include classes or categories that are not in these images, 
 *     but are newly eligible because of these image.
 *   Class +loads will be called superclass-first because of the 
 *     superclass-first nature of the connecting process.
 *   Category +load needs to be deferred until the parent class is 
 *     connected and has had its +load called.
 * 
 * Performance: all classes are read before any categories are read. 
 * Fewer categories need be pended for lack of a parent class.
 * 
 * Performance: all categories are attempted to be attached before 
 * any classes are connected. Fewer class caches need be flushed. 
 * (Unconnected classes and their respective subclasses are guaranteed 
 * to be un-messageable, so their caches will be empty.)
 * 
 * Performance: all classes are read before any classes are connected. 
 * Fewer classes need be pended for lack of a superclass.
 * 
 * Correctness: all selector and class refs are fixed before any 
 * protocol fixups or +load methods. libobjc itself contains selector 
 * and class refs which are used in protocol fixup and +load.
 * 
 * Correctness: +load methods are scheduled in bottom-up link order. 
 * This constraint is in addition to superclass order. Some +load 
 * implementations expect to use another class in a linked-to library, 
 * even if the two classes don't share a direct superclass relationship.
 * 
 * Correctness: all classes are scanned for +load before any categories 
 * are attached. Otherwise, if a category implements +load and its class 
 * has no class methods, the class's +load scan would find the category's 
 * +load method, which would then be called twice.
 *
 * Correctness: pended class refs are not fixed up until the class is 
 * connected. Classes with missing weak superclasses remain unconnected. 
 * Class refs to classes with missing weak superclasses must be nil. 
 * Therefore class refs to unconnected classes must remain un-fixed.
 * 
 **********************************************************************/
