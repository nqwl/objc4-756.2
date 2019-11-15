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
        NSMutableDictionary *dic = [[NSMutableDictionary alloc] init];
        [dic setObject:@"" forKey:@"1"];
        NSLog(@"%@",dic);

    }
    return 0;
}
