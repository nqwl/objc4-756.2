//
//  Person.h
//  objc-debug
//
//  Created by 陈辉 on 2019/10/16.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface Person : NSObject
@property (nonatomic, copy) NSString *name;
- (void)test;
- (void)run;
@end

NS_ASSUME_NONNULL_END
