#ifndef JJ_Header_h
#define JJ_Header_h

//JJ内存搜索引擎(专为H5GG定制)

/* 一定要加上-fvisibility=hidden编译参数, 否则容易崩溃 */

#pragma GCC diagnostic ignored "-Wdeprecated-register"

#define JJLog(...) //NSLog(__VA_ARGS__)

#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unordered_map>
#include <ext/hash_map>
#include <vector>
#include <map>
#include <set>

#include "vmtag.h"

using namespace std;

// 内存优化常量
#define MAX_SEARCH_RESULTS 100000  // 最大搜索结果数量限制，防止内存耗尽
#define REGION_CACHE_SIZE 64 * 1024 * 1024  // 单次加载区域最大大小64MB
#define GROUP_SEARCH_TIMEOUT_SECONDS 60  // 联合搜索超时时间（秒）

extern "C" kern_return_t mach_vm_region
(
     vm_map_t target_task,
     mach_vm_address_t *address,
     mach_vm_size_t *size,
     vm_region_flavor_t flavor,
     vm_region_info_t info,
     mach_msg_type_number_t *infoCnt,
     mach_port_t *object_name
 );

extern "C" kern_return_t mach_vm_protect
(
 vm_map_t target_task,
 mach_vm_address_t address,
 mach_vm_size_t size,
 boolean_t set_maximum,
 vm_prot_t new_protection
 );

enum JJ_Search_Type
{
    JJ_Search_Type_Error,
    
    JJ_Search_Type_Double,
    JJ_Search_Type_ULong,
    JJ_Search_Type_SLong,
    JJ_Search_Type_Float,
    JJ_Search_Type_UInt,
    JJ_Search_Type_SInt,
    JJ_Search_Type_UShort,
    JJ_Search_Type_SShort,
    JJ_Search_Type_UByte,
    JJ_Search_Type_SByte,
    
    JJ_Search_Type_Max,
};

const int JJ_Search_Type_Len[] = {0,8,8,8,4,4,4,2,2,1,1};


typedef struct _result_region{
    uint64_t region_base;
    size_t region_size;
    vector<uint32_t> slides;
    vector<int8_t> types;
    
    _result_region(uint64_t base, size_t size) {
        region_base = base;
        region_size = size;
    }
} result_region;

typedef struct _result{
    vector<result_region*> regions;
    size_t count;
} Result;

typedef struct _addrRange{
    uint64_t start;
    uint64_t end;
} AddrRange;

// 联合搜索数据结构
typedef struct _groupItem {
    uint8_t valuebuf[16];  // 支持最大16字节（F64/I64）
    int type;               // 值类型
    int len;                // 值长度
    
    _groupItem() {
        memset(valuebuf, 0, sizeof(valuebuf));
        type = 0;
        len = 0;
    }
} GroupItem;

// 联合搜索参数
typedef struct _groupSearchParams {
    vector<GroupItem> items;      // 搜索项列表
    int totalRange;               // 总范围（步长）
    int anchorIndex;              // 锚点索引（用于优化的值）
    AddrRange range;              // 搜索内存范围
    
    _groupSearchParams() {
        totalRange = 0;
        anchorIndex = 0;
        range.start = 0;
        range.end = 0;
    }
} GroupSearchParams;

// 联合搜索结果
typedef struct _groupResult {
    uint64_t baseAddress;
    vector<uint32_t> slides;
    int matchCount;
    
    _groupResult(uint64_t base) {
        baseAddress = base;
        matchCount = 0;
    }
} GroupResult;

class JJMemoryEngine
{
    mach_port_t task;
    Result *result;
    map<uint64_t,uint64_t> regions;
    bool firstScanDone;
    float float_tolerance;
    int lastNumberType;
    bool memoryLimitReached;  // 内存限制标志，防止过度分配
    
    void freeResults()
    {
        if(result) {
            for(int i = 0; i < result->regions.size(); i++){
                if(result->regions[i]) {
                    result->regions[i]->slides.clear();
                    result->regions[i]->slides.shrink_to_fit();
                    result->regions[i]->types.clear();
                    result->regions[i]->types.shrink_to_fit();
                    delete result->regions[i];
                    result->regions[i] = NULL;
                }
            }
            result->regions.clear();
            result->regions.shrink_to_fit();
            delete result;
            result = NULL;
        }
        // 重置内存限制标志
        this->memoryLimitReached = false;
    }
    
    bool readMemory(void* buf, uint64_t addr, size_t len)
    {
        vm_size_t size = 0;
        kern_return_t kr = vm_read_overwrite(this->task, (vm_address_t)addr, len, (vm_address_t)buf, &size);
        if(kr != KERN_SUCCESS || size!=len)
        {
            NSLog(@"readMemory failed! %p %x, (%d)%s", addr, len, kr, mach_error_string(kr));
            return false;
        }
        
        return true;
    }
    
    bool writeMemory(void* address,void *target, size_t len)
    {
        kern_return_t error = vm_write(this->task, (vm_address_t)address, (vm_offset_t)target, (mach_msg_type_number_t)len);
        if(error != KERN_SUCCESS)
        {
            NSLog(@"writeMemory failed! %p %x", address, len);
            return false;
        }
        
        return true;
    }
    
    uint64_t ScanData(uint64_t buffer, uint64_t size, void* target, int type)
    {
        int len = JJ_Search_Type_Len[type];
        
        register uint64_t p=buffer;
        uint64_t end = buffer + size - len;
        
        switch(type)
        {
            case JJ_Search_Type_Float: {
                register float value_up =  *((float*)target+1) + this->float_tolerance;
                register float value_down = *(float*)target - this->float_tolerance;
                while(p<=end) {
                    register float v = *(float*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_Double: {
                register double value_up =  *((double*)target+1) + this->float_tolerance;
                register double value_down = *(double*)target - this->float_tolerance;
                while(p<=end) {
                    register double v = *(double*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_SByte: {
                register int8_t value_up =  *((int8_t*)target+1);
                register int8_t value_down = *(int8_t*)target;
                while(p<=end) {
                    register int8_t v = *(int8_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_UByte: {
                register uint8_t value_up =  *((uint8_t*)target+1);
                register uint8_t value_down = *(uint8_t*)target;
                while(p<=end) {
                    register uint8_t v = *(uint8_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_SShort: {
                register int16_t value_up =  *((int16_t*)target+1);
                register int16_t value_down = *(int16_t*)target;
                while(p<=end) {
                    register int16_t v = *(int16_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_UShort: {
                register uint16_t value_up =  *((uint16_t*)target+1);
                register uint16_t value_down = *(uint16_t*)target;
                while(p<=end) {
                    register uint16_t v = *(uint16_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_SInt: {
                register int32_t value_up =  *((int32_t*)target+1);
                register int32_t value_down = *(int32_t*)target;
                while(p<=end) {
                    register int32_t v = *(int32_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_UInt: {
                register uint32_t value_up =  *((uint32_t*)target+1);
                register uint32_t value_down = *(uint32_t*)target;
                while(p<=end) {
                    register uint32_t v = *(uint32_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_SLong: {
                register int64_t value_up =  *((int64_t*)target+1);
                register int64_t value_down = *(int64_t*)target;
                while(p<=end) {
                    register int64_t v = *(int64_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
                
            case JJ_Search_Type_ULong: {
                register uint64_t value_up =  *((uint64_t*)target+1);
                register uint64_t value_down = *(uint64_t*)target;
                while(p<=end) {
                    register uint64_t v = *(uint64_t*)p;
                    if(v>=value_down && v<=value_up) break;
                    p+=len;
                }
            } break;
        }
        
        return p<=end ? p : 0;
    }
    
    void* loadRegion(uint64_t base, uint64_t* psize, bool* remapped)
    {
        size_t size=*psize;
        for(int s=0; s<size; s+=PAGE_SIZE)
        {
            uint64_t a=0;
            if(vm_read_overwrite(this->task, (vm_address_t)(base+s), sizeof(a), (vm_address_t)&a, (vm_size_t*)&a)!=KERN_SUCCESS)
            {
                size = s;
                break;
            }
        }
        
        if(!size) return NULL;
        
        *psize = size;
        
        vm_address_t buffer=0;
        
        vm_prot_t cur_prot=0;
        vm_prot_t max_prot=0;
        
        do {
            
            if(this->task==mach_task_self())
            {
                mach_port_t object_name;
                mach_vm_size_t region_size=size;
                mach_vm_address_t region_base = base;
                
                vm_region_extended_info info={0};
                mach_msg_type_number_t info_cnt = VM_REGION_EXTENDED_INFO_COUNT;
                vm_region_flavor_t flavor = VM_REGION_EXTENDED_INFO;
                
                kern_return_t kr = mach_vm_region(this->task, &region_base, &region_size,
                                                      flavor, (vm_region_info_t)&info, &info_cnt, &object_name);
                if(kr==KERN_SUCCESS && info.user_tag==VM_MEMORY_MALLOC_NANO) {
                    *remapped = false;
                    buffer = base;
                    break;
                }
            }
            
            kern_return_t kr = vm_remap(mach_task_self(), &buffer, size, 0, VM_FLAGS_ANYWHERE,
                                        this->task, base, false, &cur_prot, &max_prot, VM_INHERIT_NONE);
            
            if(kr!=KERN_SUCCESS) {
                NSLog(@"read mem failed! %p %x, %d %s", base, size, kr, mach_error_string(kr));
                if(kr==KERN_NO_SPACE)
                    throw bad_alloc();
            } else {
                *remapped = true;
            }
            
        } while(0);
        
        NSLog(@"loadRegion[%d] %p=>%p %x,%x,%x", *remapped, base, buffer, size, cur_prot, max_prot);
        return (void*)buffer;
    }
    
    void unloadRegion(void* buffer, uint64_t size, bool remapped)
    {
        if(buffer&&remapped) {
            NSLog(@"unloadRegion %p %x", buffer, size);
            vm_deallocate(mach_task_self(), (vm_address_t)buffer, size);
        }
    }
    
    void ScanRegion(AddrRange range, uint64_t base, uint64_t size, void* target, int type)
    {
        int len = JJ_Search_Type_Len[type];
        
        // 内存优化：如果已达到结果数量限制，跳过扫描
        if(this->memoryLimitReached) {
            return;
        }
        
        result_region* newRegion = NULL;
        
        bool remapped;
        void* buffer = loadRegion(base, &size, &remapped);
        
        if(buffer)
        {
            uint64_t pcurdata = (uint64_t)buffer;
            uint64_t left_size = size;
            while(left_size >= len)
            {
                // 内存优化：限制结果数量，防止内存耗尽
                if(this->result->count >= MAX_SEARCH_RESULTS) {
                    this->memoryLimitReached = true;
                    NSLog(@"Search results reached limit: %d", MAX_SEARCH_RESULTS);
                    break;
                }
                
                uint64_t pfound = ScanData(pcurdata, left_size, target, type);
                if(!pfound) break;
                
                uint32_t slide = (uint32_t)(pfound - (uint64_t)buffer);
                
                if((base+slide)<range.start || (base+slide)>=range.end) break;
                
                if(!newRegion)
                    newRegion = new result_region(base,size);
                
                newRegion->slides.push_back(slide);
                this->result->count++;
                
                pcurdata = pfound + len;
                left_size = (uint64_t)buffer+size - pcurdata;
            }
            
        }
        
        if(newRegion) {
            newRegion->slides.shrink_to_fit();
            this->result->regions.push_back(newRegion);
        }
        
        unloadRegion(buffer, size, remapped);
    }
    
    
    void FirstScan(AddrRange range, void* target, int type)
    {
        int len = JJ_Search_Type_Len[type];
        
        size_t stack_size=pthread_get_stacksize_np(pthread_self());
        size_t stack_addr=(size_t)pthread_get_stackaddr_np(pthread_self());
        size_t stack_end = stack_addr + stack_size;
        NSLog(@"stack=%p %x => %p", stack_addr, stack_size, stack_end);
        
        vm_size_t region_size=0;
        vm_address_t region_base = range.start;

        
        natural_t depth = 1;
        
        while(region_base < range.end) {
            region_base += region_size;
            
            struct vm_region_submap_info_64 info={0};
            mach_msg_type_number_t info_cnt = VM_REGION_SUBMAP_INFO_COUNT_64;
            
            kern_return_t kr = vm_region_recurse_64(this->task, &region_base, &region_size,
                                              &depth, (vm_region_info_t)&info, &info_cnt);
            
            if(kr != KERN_SUCCESS) {
                NSLog(@"mach_vm_region failed on %p for %d,%s", region_base, kr, mach_error_string(kr));
                break;
            }
            
            const char* tag = name_for_tag(info.user_tag);
            NSLog(@"found region %p %x [%d/%d], %x, %s", region_base, region_size, info.is_submap, depth, info.protection, tag);
            
            if(info.is_submap) {
                region_size=0;
                depth++;
                continue;
            }
            
            uint64_t region_end = region_base+region_size;
            
            if(this->task==mach_task_self()) {
                if((stack_addr>=region_base && stack_addr<region_end)
                   || (stack_end>region_base && stack_addr<=region_end)) {
                    NSLog(@"skip stack region!");
                    continue;
                }
            }
            
            if(!(info.protection & VM_PROT_WRITE)) {
                NSLog(@"skip readlony region!");
                continue;
            }
                
            this->regions[region_base] = region_size;
        }
        
        int i=0;
        for(auto region : this->regions) {
            NSLog(@"handle region[%d/%d] %p %x [%d]",i++, this->regions.size(),
                  region.first, region.second, this->result->count);
            ScanRegion(range, region.first, region.second, target, type);
        }
        
        this->result->regions.shrink_to_fit();
    }
    
    void ScanAgain(AddrRange range, void* target, int type)
    {
        int len = JJ_Search_Type_Len[type];
        
        size_t newCount = 0;
        
        for(int i=0; i<this->result->regions.size(); i++)
        {
            result_region* region = this->result->regions[i];
            
            NSLog(@"handle region [%d/%d]%d %p %x", i, this->result->regions.size(), region->slides.size(),
                  region->region_base, region->region_size);
            
            if((region->region_base+region->region_size)<range.start || region->region_base>range.end)
                continue;
            
            result_region* newRegion = NULL;
            
            bool remapped; uint64_t mapsize=region->region_size;
            void* buffer = loadRegion(region->region_base, &mapsize, &remapped);
            if(buffer) for(int j=0; j<region->slides.size(); j++)
            {
                UInt64 address = (UInt64)region->region_base + (UInt64)region->slides[j];
                void* pvalue = (void*)((UInt64)buffer + (UInt64)region->slides[j]);
                
                //NSLog(@"handle slide [%d] %p %x : %llX", j, address, region->slide[j], *(UInt64*)pvalue);
                
                if(address>=range.start && address<range.end &&
                   ScanData((uint64_t)pvalue, len, target, type))
                {
                    if(!newRegion)
                        newRegion = new result_region(region->region_base,region->region_size);
                        
                    //NSLog(@"found %p %x", region->region_base, region->slide[j]);
                    
                    newRegion->slides.push_back(region->slides[j]);
                    newCount++;
                }
            } else {
                NSLog(@"read mem failed! [%d] %p %x", i, region->region_base, region->region_size);
            }
            
            //BUG=一定要在delete old region之前, 不然这里size不可预料了
            unloadRegion(buffer, mapsize, remapped);
            
            delete this->result->regions[i];
            this->result->regions[i] = newRegion;
            if(newRegion) newRegion->slides.shrink_to_fit();
        }
        
        
        this->result->regions.erase(
                                    remove(this->result->regions.begin(),this->result->regions.end(), (result_region*)NULL), this->result->regions.end());
        
        this->result->regions.shrink_to_fit();
        
        this->result->count = newCount;
    }
    
public:
    JJMemoryEngine(mach_port_t task){
        this->task = task;
        
        this->result = new Result;
        this->result->count = 0;
        
        this->firstScanDone = false;
        this->float_tolerance = 0.0;
        this->lastNumberType = 0;
        this->memoryLimitReached = false;  // 初始化内存限制标志
    }
    
    ~JJMemoryEngine(){
        freeResults();
    }
    
    void SetFloatTolerance(float d)
    {
        this->float_tolerance = d;
    }
    
    void JJScanMemory(AddrRange range, void* target, int type)
    {
        if(type<=0 || type>=JJ_Search_Type_Max) return;
        
        this->lastNumberType = type;
        
        if(this->firstScanDone) {
            ScanAgain(range, target, type);
        } else {
            FirstScan(range, target, type);
            this->firstScanDone = true;
        }
    }

    void JJNearBySearch(size_t range, void *target, int type)
    {
        if(type<=0 || type>=JJ_Search_Type_Max) return;
        
        int len = JJ_Search_Type_Len[type];
        
        size_t newCount = 0;
        
        range -= range%len;
        range += len;
        
        for(int i=0; i<this->result->regions.size(); i++)
        {
            result_region* region = this->result->regions[i];
            
            bool hasType = region->types.size()>0;
            bool needType = hasType || type!=this->lastNumberType;
            
            NSLog(@"handle region [%d/%d] %p,%x : %d", i, this->result->regions.size(),
                  region->region_base, region->region_size, region->slides.size());
            
            result_region* newRegion = NULL;
            
            int lastold = 0;
            
            long lastpos = 0;
            
            
            bool remapped; uint64_t mapsize=region->region_size;
            void* buffer = loadRegion(region->region_base, &mapsize, &remapped);
            if(buffer) for(int j=0; j<region->slides.size(); j++)
            {
                map<uint32_t,int8_t> matched;
                
                uint32_t curslide = region->slides[j];
                long range_start = curslide - range;
                long range_end = curslide + range;
                
                if(range_start < 0) range_start = 0;
                if(range_end > region->region_size) range_end=region->region_size;
                
                if(lastpos > range_start)
                    range_start = lastpos;
                
                lastpos = range_end;
                
                uint64_t data = (uint64_t)buffer + range_start;
                size_t size = range_end - range_start;
                
                JJLog(@"%x[%d]%x [%x %x]", range, j, curslide, range_start, range_end);
                //assert(size>=0 && size<=range*2);
                
                int foundcount = 0;
                uint32_t foundfirst = 0;
                uint32_t foundlast = 0;
                
                uint64_t pcurdata = data;
                uint64_t left_size = size;
                while(left_size >= len)
                {
                    uint64_t pfound = ScanData(pcurdata, left_size, target, type);
                    if(!pfound) break;
                    
                    
                    uint32_t slide = (uint32_t)(pfound - (uint64_t)buffer);
                    
                    JJLog(@"found %x", slide);
                    
                    matched[slide] = type;
                    
                    if(foundcount==0) foundfirst = slide;
                    foundlast = slide;
                    foundcount++;
                    
                    pcurdata = pfound + len;
                    left_size = (uint64_t)data+size - pcurdata;
                }
                
                
                if(foundcount) for(int o=lastold; o<region->slides.size(); o++) {
                    
                    uint32_t oldslide = region->slides[o];
                    
                    long first_down = (foundfirst-range);
                    long first_up = (foundfirst+range);
                    long last_down = (foundlast-range);
                    long last_up = (foundlast+range);
                    
                    //assert(last_down<first_up);
                    
                    if((oldslide>first_down && oldslide<first_up) || (oldslide>last_down && oldslide<last_up))
                    {
                        JJLog(@"old %d %d [%d] %x", j, lastold, o, oldslide);
                        
                        matched[oldslide] = hasType ? region->types[o] : this->lastNumberType;
                        
                        lastold = o+1;
                    }
                }
                
                if(matched.size()) {
                    
                    if(!newRegion)
                        newRegion = new result_region(region->region_base, region->region_size);
                    
                    for(auto it = matched.begin(); it != matched.end(); ++it) {
                        newRegion->slides.push_back(it->first);
                        if(needType) newRegion->types.push_back(it->second);
                    }
                    
                    newCount += matched.size();
                    
                    JJLog(@"nearby search region %p count=%d=>%d", region->region_base, matched.size(), newCount);
                }
                
            } else {
                NSLog(@"read mem failed! [%d] %p %x", i, region->region_base, region->region_size);
            }
            
            //BUG=一定要在delete old region之前, 不然这里size不可预料了
            unloadRegion(buffer, mapsize, remapped);
            
            delete this->result->regions[i];
            this->result->regions[i] = newRegion;
            if(newRegion) {
                newRegion->slides.shrink_to_fit();
                newRegion->types.shrink_to_fit();
            }
        }
        
        this->result->regions.erase(
                                    remove(this->result->regions.begin(),this->result->regions.end(), (result_region*)NULL), this->result->regions.end());
        
        this->result->regions.shrink_to_fit();
        
        this->result->count = newCount;
    }
    
    bool JJReadMemory(void* buf, uint64_t addr, int type)
    {
        //NSLog(@"JJReadMemory %p %d", addr, type);
        
        if(type<=0 || type>=JJ_Search_Type_Max) return false;
        
        int len = JJ_Search_Type_Len[type];
        
        return readMemory(buf, addr, len);
    }
    
    bool JJWriteMemory(void* address,void *target, int type)
    {
        if(type<=0 || type>=JJ_Search_Type_Max) return false;
        
        int len = JJ_Search_Type_Len[type];
        
        mach_port_t object_name;
        mach_vm_size_t region_size=0;
        mach_vm_address_t region_base = (uint64_t)address;
        
        vm_region_basic_info_data_64_t info = {0};
        mach_msg_type_number_t info_cnt = VM_REGION_BASIC_INFO_COUNT_64;
        
        
        kern_return_t kr = mach_vm_region(this->task, &region_base, &region_size,
                                              VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &info_cnt, &object_name);
        if(kr != KERN_SUCCESS) {
            NSLog(@"mach_vm_region failed! %p", region_base);
            return false;
        }
        
        vm_address_t base = 0;
        if(!(info.protection & VM_PROT_WRITE)) {
            NSLog(@"unwritable region %p %x : %x", region_base, region_size, info.protection);
            base = (uint64_t)address & ~PAGE_MASK;
            //c1越狱这里可能失败, 不能同时rwx??? c1这里返回成功但是实际上并没有成功!!!!
            kr = mach_vm_protect(this->task, base, PAGE_SIZE, false, info.protection|VM_PROT_WRITE|VM_PROT_COPY);
            if(kr != KERN_SUCCESS) {
                NSLog(@"vm_protect failed! kr=%d [%p %x] : %x", kr, base, PAGE_SIZE, info.protection);
                
                kr = mach_vm_protect(this->task, base, PAGE_SIZE, false, VM_PROT_READ|VM_PROT_WRITE|VM_PROT_COPY);
                if(kr != KERN_SUCCESS) {
                    NSLog(@"vm_protect failed2! kr=%d [%p %x] : %x", kr, base, PAGE_SIZE, info.protection);
                    
                    //NSLog(@"mprotect=%d, %d, %s", mprotect((void*)base, PAGE_SIZE, info.protection|VM_PROT_WRITE), errno, strerror(errno));
                    
                    return false;
                }
            }
        }
        
        bool result = writeMemory(address, target, len);
        
        if(!result && base) {
            
            kr = mach_vm_protect(this->task, base, PAGE_SIZE, false, VM_PROT_READ|VM_PROT_WRITE|VM_PROT_COPY);
            
            if(kr != KERN_SUCCESS) {
                NSLog(@"vm_protect again failed! kr=%d [%p %x] : %x", kr, base, PAGE_SIZE, info.protection);
            } else {
                result = writeMemory(address, target, len);
            }
        }
        
        if(base)
            vm_protect(this->task, base, PAGE_SIZE, false, info.protection);
        
        return result;
    }
    
    int JJWriteAll(void * target, int type)
    {
        if(type<=0 || type>=JJ_Search_Type_Max) return 0;
        
        int len = JJ_Search_Type_Len[type];
        
        int count=0;
        for(int i=0; i<this->result->regions.size(); i++)
        {
            result_region* region = result->regions[i];
            for(int j=0; j<region->slides.size(); j++)
            {
                uint64_t address = region->region_base + region->slides[j];
                if(writeMemory((void*)address, target, len))
                    count++;
            }
        }
        return count;
    }
     
    size_t getResultsCount()
    {
        return this->result->count;
    }
    
    vector<void*> getResults(size_t count, size_t skip=0)
    {
        vector<void*> results;
        int index=0;
        for(int i=0; i<this->result->regions.size(); i++)
        {
            result_region* region = result->regions[i];
            
            if((index + region->slides.size()) <= skip) {
                index += region->slides.size();
                continue;
            }
            
            for(int j=0; j<region->slides.size(); j++)
            {
                if(index>=skip && (index-skip)<count) {
                    uint64_t address = region->region_base + region->slides[j];
                    results.push_back((void*)address);
                }
                index++;
            }
        }
        return results;
    }
    
    map<void*,int8_t> getResultsAndTypes(int count, int skip=0)
    {
        map<void*,int8_t> results;
        int index=0;
        for(int i=0; i<this->result->regions.size(); i++)
        {
            result_region* region = this->result->regions[i];
            auto hasTypes = region->types.size();
            
            if((index + region->slides.size()) <= skip) {
                index += region->slides.size();
                continue;
            }
            
            for(int j=0; j<region->slides.size(); j++)
            {
                if(index>=skip && (index-skip)<count) {
                    uint64_t address = region->region_base + region->slides[j];
                    results[(void*)address] = hasTypes ? this->result->regions[i]->types[j] : 0;
                }
                index++;
            }
        }
        return results;
    }
    
    // 比较指定地址的值是否与给定项匹配
    bool compareValueAtAddress(uint64_t addr, GroupItem* item)
    {
        uint8_t readBuf[16] = {0};
        if(!readMemory(readBuf, addr, item->len)) {
            return false;
        }
        
        // 根据类型比较
        switch(item->type) {
            case JJ_Search_Type_Float: {
                float v1 = *(float*)item->valuebuf;
                float v2 = *(float*)readBuf;
                float diff = fabs(v1 - v2);
                if(diff <= this->float_tolerance) return true;
            } break;
            case JJ_Search_Type_Double: {
                double v1 = *(double*)item->valuebuf;
                double v2 = *(double*)readBuf;
                double diff = fabs(v1 - v2);
                if(diff <= this->float_tolerance) return true;
            } break;
            case JJ_Search_Type_SByte: {
                if(*(int8_t*)readBuf == *(int8_t*)item->valuebuf) return true;
            } break;
            case JJ_Search_Type_UByte: {
                if(*(uint8_t*)readBuf == *(uint8_t*)item->valuebuf) return true;
            } break;
            case JJ_Search_Type_SShort: {
                if(*(int16_t*)readBuf == *(int16_t*)item->valuebuf) return true;
            } break;
            case JJ_Search_Type_UShort: {
                if(*(uint16_t*)readBuf == *(uint16_t*)item->valuebuf) return true;
            } break;
            case JJ_Search_Type_SInt: {
                if(*(int32_t*)readBuf == *(int32_t*)item->valuebuf) return true;
            } break;
            case JJ_Search_Type_UInt: {
                if(*(uint32_t*)readBuf == *(uint32_t*)item->valuebuf) return true;
            } break;
            case JJ_Search_Type_SLong: {
                if(*(int64_t*)readBuf == *(int64_t*)item->valuebuf) return true;
            } break;
            case JJ_Search_Type_ULong: {
                if(*(uint64_t*)readBuf == *(uint64_t*)item->valuebuf) return true;
            } break;
        }
        return false;
    }
    
    // 在指定范围内查找匹配项的地址
    vector<uint64_t> findValueInRange(uint64_t startAddr, uint64_t endAddr, GroupItem* item)
    {
        vector<uint64_t> matches;
        uint64_t addr = startAddr;
        
        while(addr < endAddr && matches.size() < 10000) {  // 限制每轮匹配数量
            if(compareValueAtAddress(addr, item)) {
                matches.push_back(addr);
            }
            addr += item->len;
        }
        
        return matches;
    }
    
    // 计算某个值在内存中的匹配数量（用于锚点选择优化）
    size_t countMatchesInRange(AddrRange range, GroupItem* item)
    {
        size_t count = 0;
        size_t checkedSize = 0;
        
        // 抽样统计，避免全量扫描消耗太多时间
        // 对于小于16KB的区域进行全量检查
        if(range.end - range.start <= 16 * 1024) {
            for(uint64_t addr = range.start; addr < range.end; addr += item->len) {
                uint8_t readBuf[16] = {0};
                if(readMemory(readBuf, addr, item->len)) {
                    if(memcmp(readBuf, item->valuebuf, item->len) == 0) {
                        count++;
                    }
                }
            }
            return count;
        }
        
        // 对于大区域，抽样统计（每16字节检查一次）
        size_t sampleStep = 16;
        for(uint64_t addr = range.start; addr < range.end; addr += sampleStep) {
            uint8_t readBuf[16] = {0};
            if(readMemory(readBuf, addr, item->len)) {
                if(memcmp(readBuf, item->valuebuf, item->len) == 0) {
                    count++;
                }
            }
            checkedSize += sampleStep;
            // 限制检查范围，最多检查4MB
            if(checkedSize >= 4 * 1024 * 1024) {
                break;
            }
        }
        
        // 按比例估算总数
        if(range.end - range.start > 4 * 1024 * 1024) {
            double ratio = (double)(range.end - range.start) / checkedSize;
            count = (size_t)(count * ratio);
        }
        
        return count;
    }
    
    // 联合搜索：使用给定参数搜索多个值的组合
    void JJGroupSearch(GroupSearchParams* params)
    {
        if(params->items.size() < 2) {
            NSLog(@"JJGroupSearch: need at least 2 items");
            return;
        }
        
        int itemCount = (int)params->items.size();
        int anchorIdx = params->anchorIndex;
        int range = params->totalRange;
        AddrRange searchRange = params->range;
        
        NSLog(@"JJGroupSearch: itemCount=%d, anchorIdx=%d, range=%d", itemCount, anchorIdx, range);
        
        // 记录开始时间用于超时检测
        time_t startTime = time(NULL);
        
        // 优化锚点选择：如果指定了锚点索引则使用它，否则自动选择
        if(anchorIdx < 0 || anchorIdx >= itemCount) {
            // 自动选择锚点 - 选择匹配数量最少的值作为锚点
            size_t minMatches = SIZE_MAX;
            int bestAnchor = 0;
            
            NSLog(@"JJGroupSearch: auto selecting best anchor...");
            for(int i = 0; i < itemCount; i++) {
                size_t matches = countMatchesInRange(searchRange, &params->items[i]);
                NSLog(@"JJGroupSearch: item[%d] estimated matches=%zu", i, matches);
                if(matches < minMatches) {
                    minMatches = matches;
                    bestAnchor = i;
                }
            }
            anchorIdx = bestAnchor;
            NSLog(@"JJGroupSearch: selected anchor=%d with ~%zu matches", anchorIdx, minMatches);
        }
        
        // 获取锚点项
        GroupItem* anchorItem = &params->items[anchorIdx];
        
        // 第一阶段：查找锚点值的所有匹配地址
        vector<uint64_t> anchorMatches = findValueInRange(searchRange.start, searchRange.end, anchorItem);
        NSLog(@"JJGroupSearch: anchor matches=%zu", anchorMatches.size());
        
        if(anchorMatches.empty()) {
            return;
        }
        
        // 限制锚点匹配数量，防止内存溢出
        if(anchorMatches.size() > 10000) {
            NSLog(@"JJGroupSearch: too many anchor matches, limiting to 10000");
            anchorMatches.resize(10000);
        }
        
        // 第二阶段：对每个锚点匹配，验证其他值
        for(size_t i = 0; i < anchorMatches.size() && !this->memoryLimitReached; i++) {
            // 检查超时
            time_t currentTime = time(NULL);
            if(currentTime - startTime >= GROUP_SEARCH_TIMEOUT_SECONDS) {
                NSLog(@"JJGroupSearch: timeout after %d seconds", GROUP_SEARCH_TIMEOUT_SECONDS);
                break;
            }
            uint64_t anchorAddr = anchorMatches[i];
            bool allMatch = true;
            uint64_t baseAddress = 0;  // 记录第一个值的地址（用于结果输出）
            
            // 检查是否使用滑动窗口模式（所有值连续）
            if(range > 0) {
                // 计算第一个值的基准地址
                // 如果锚点不是第一个值，需要向前偏移（减去锚点之前所有值的长度）
                baseAddress = anchorAddr;
                for(int j = 0; j < anchorIdx; j++) {
                    baseAddress -= params->items[j].len;
                }
                
                // 验证基准地址是否在搜索范围内
                if(baseAddress < searchRange.start || baseAddress + range > searchRange.end) {
                    continue;
                }
                
                // 按用户输入的顺序验证每个值
                for(int j = 0; j < itemCount; j++) {
                    GroupItem* item = &params->items[j];
                    uint64_t targetAddr = baseAddress;
                    
                    // 计算当前值相对于基准地址的偏移
                    for(int k = 0; k < j; k++) {
                        targetAddr += params->items[k].len;
                    }
                    
                    if(!compareValueAtAddress(targetAddr, item)) {
                        allMatch = false;
                        break;
                    }
                }
                
                if(allMatch) {
                    // 找到一个匹配
                    if(!this->result->regions.empty() && 
                       this->result->regions.back()->region_base == baseAddress) {
                        // 同一个地址，添加到现有region
                        this->result->regions.back()->slides.push_back(0);
                    } else {
                        result_region* newRegion = new result_region(baseAddress, range);
                        newRegion->slides.push_back(0);
                        this->result->regions.push_back(newRegion);
                    }
                    this->result->count++;
                    
                    if(this->result->count >= MAX_SEARCH_RESULTS) {
                        this->memoryLimitReached = true;
                        NSLog(@"JJGroupSearch: reached max results limit");
                        break;
                    }
                }
            } else {
                // 非连续模式：每个值在各自的位置（范围限制内）
                // 计算基准地址
                baseAddress = anchorAddr;
                for(int j = 0; j < anchorIdx; j++) {
                    baseAddress -= params->items[j].len;
                }
                
                if(baseAddress < searchRange.start) {
                    continue;
                }
                
                for(int j = 0; j < itemCount; j++) {
                    GroupItem* item = &params->items[j];
                    uint64_t targetAddr = baseAddress;
                    
                    for(int k = 0; k < j; k++) {
                        targetAddr += params->items[k].len;
                    }
                    
                    if(targetAddr >= searchRange.end || !compareValueAtAddress(targetAddr, item)) {
                        allMatch = false;
                        break;
                    }
                }
                
                if(allMatch) {
                    result_region* newRegion = new result_region(baseAddress, 0);
                    newRegion->slides.push_back(0);
                    this->result->regions.push_back(newRegion);
                    this->result->count++;
                }
            }
        }
        
        // 优化：释放向量多余容量
        for(auto region : this->result->regions) {
            region->slides.shrink_to_fit();
        }
        this->result->regions.shrink_to_fit();
        
        NSLog(@"JJGroupSearch: final result count=%zu", this->result->count);
    }
};

#endif /* JJ_Header_h */
