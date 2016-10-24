//
// Created by zhangyutong926 on 10/25/16.
//

#ifndef CWOJ_DAEMON_UTIL_H
#define CWOJ_DAEMON_UTIL_H

template<typename B, typename T>
inline T *FromOffset(B *base, T B:: *offset) {
    unsigned char *baseInter = (unsigned char *) base;
    int offsetValue = reinterpret_cast<int>(*(void **)(&offset));
    baseInter += offsetValue;
    return reinterpret_cast<T *>(baseInter);
}

#endif //CWOJ_DAEMON_UTIL_H
