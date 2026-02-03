#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

typedef NS_ENUM(NSInteger, MIValueType) {
    MIValueType_Int8,
    MIValueType_UInt8,
    MIValueType_Int16,
    MIValueType_UInt16,
    MIValueType_Int32,
    MIValueType_UInt32,
    MIValueType_Int64,
    MIValueType_UInt64,
    MIValueType_Float,
    MIValueType_Double,
    MIValueType_Pointer,
    MIValueType_Hex,
    MIValueType_ASCII
};

@interface MemoryInspectorResult : NSObject
@property (nonatomic, assign) uint64_t baseAddress;
@property (nonatomic, strong) NSData *bytes; // raw bytes read
@end

@interface MemoryInspector : NSObject

// enginePtr: 指向 JJMemoryEngine 的实例指针（void*）
- (instancetype)initWithJJEnginePointer:(void *)enginePtr;

// 读取 baseAddr 开始的 length 字节到 result（返回 YES 表示读取成功）
- (BOOL)readBytesAt:(uint64_t)baseAddr length:(size_t)length outResult:(MemoryInspectorResult **)result;

// 把 result.bytes 中从 offset 处按 type 和 pointerSize(4/8) 和 littleEndian(BOOL) 解码成字符串
- (NSString *)stringForResult:(MemoryInspectorResult *)result offset:(size_t)offset type:(MIValueType)type pointerSize:(int)ptrSize littleEndian:(BOOL)le;

@end
