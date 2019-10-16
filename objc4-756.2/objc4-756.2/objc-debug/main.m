//
//  main.m
//  objc-debug
//
//  Created by Cooci on 2019/10/9.
//

#import <Foundation/Foundation.h>
#import "Person.h"
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
//        Person *object = [[Person alloc] init];
        printf("TARGET_OS_MAC: %d\n",TARGET_OS_MAC);
        printf("TARGET_OS_WIN32: %d\n",TARGET_OS_WIN32);
        NSObject *object = [[NSObject alloc] init];
//        NSObject *object = [NSObject alloc];
        NSLog(@"Hello, World! %@",object);

    }
    return 0;
}
