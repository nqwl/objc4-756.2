//
//  Person.m
//  objc-debug
//
//  Created by 陈辉 on 2019/10/16.
//

#import "Person.h"
#import <objc/runtime.h>
#import "Student.h"
@implementation Person

+ (void)initialize {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [self methodExchange];
    });
}
+ (void)methodExchange {
    Method eatMethod = class_getInstanceMethod(self, @selector(eat));
    Method runMethod = class_getInstanceMethod(self, @selector(run));
    method_exchangeImplementations(eatMethod, runMethod);
}
//void teeee(id self,SEL _cmd){
//    NSLog(@"动态解析");
//}
- (void)eat {

}
- (void)run {

}
+ (BOOL)resolveInstanceMethod:(SEL)sel {
    if (sel == @selector(test)) {
        Method method = class_getInstanceMethod(self, @selector(teeee));
        class_addMethod(self, sel, method_getImplementation(method), method_getTypeEncoding(method));
        return YES;
    }
    return [super resolveInstanceMethod:sel];
}
- (id)forwardingTargetForSelector:(SEL)aSelector {
//    if (aSelector == @selector(test)) {
//        return [Student new];
//    }
    return [super forwardingTargetForSelector:aSelector];
}
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    if (aSelector  == @selector(test)) {
        NSMethodSignature *sign = [NSMethodSignature signatureWithObjCTypes:"v@:"];
        return sign;
    }
    return [super methodSignatureForSelector:aSelector];
}
- (void)forwardInvocation:(NSInvocation *)anInvocation {
    Student *stu = [[Student alloc] init];
    anInvocation.target = stu;
    [anInvocation invoke];
}
- (void)teeee {
    NSLog(@"动态解析");
}
@end
