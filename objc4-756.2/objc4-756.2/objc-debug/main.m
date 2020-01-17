//
//  main.m
//  objc-debug
//
//  Created by Cooci on 2019/10/9.
//

#import <Foundation/Foundation.h>
#import "Person.h"
#import "Student.h"
#import <objc/runtime.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *p = [[Person alloc] init];
        [p run];
//        if ([p respondsToSelector:@selector(eat)]) {
//            [p performSelector:@selector(eat) withObject:nil];
//        }

        unsigned int count;
        Method *methodList = class_copyMethodList([Student class], &count);
        for (int i = 0; i<(int)count; i ++) {
            Method method = methodList[i];
            NSLog(@"%@",NSStringFromSelector(method_getName(method)));
        }
        free(methodList);
        NSLog(@"-------------");
        Ivar *ivars = class_copyIvarList([Person class], &count);
        for (int i = 0; i<(int)count; i ++) {
            Ivar ivar = ivars[i];
            NSLog(@"%@",[NSString stringWithUTF8String:ivar_getName(ivar)]);
        }
        free(ivars);
        NSLog(@"-------------");
        objc_property_t *properts = class_copyPropertyList([Person class], &count);
        for (int i = 0; i<(int)count; i ++) {
            objc_property_t property = properts[i];
            NSLog(@"%@",[NSString stringWithUTF8String:property_getName(property)]);
        }
        free(properts);
        //        [p performSelector:@selector(test) withObject:nil afterDelay:1];
//        NSMethodSignature *sign = [p methodSignatureForSelector:@selector(test)];
//        NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sign];
//        invocation.selector = @selector(run);
//        invocation.target = p;
//        [invocation invoke];

//        Class kid = objc_allocateClassPair([Person class], "Kid", 0);
//        objc_setAssociatedObject(kid, "name", @"jack", OBJC_ASSOCIATION_COPY_NONATOMIC);
//        objc_registerClassPair(kid);
//        NSLog(@"%@",objc_getAssociatedObject(kid, "name"));
//        objc_disposeClassPair(kid);
    }
    return 0;
}
