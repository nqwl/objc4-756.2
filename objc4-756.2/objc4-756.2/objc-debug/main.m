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
//        printf("TARGET_OS_MAC: %d\n",TARGET_OS_MAC);
//

//        Person *object = [oa init];
        NSLog(@"start!");

        for (int i = 0; i < 10e5 * 2; i++) {
            @autoreleasepool {
//                NSArray *array = [[NSArray alloc] init];
                NSArray *array = [NSArray arrayWithArray:@[@1]];
            }
        }
        NSLog(@"finished!");
    }
    return 0;
}
