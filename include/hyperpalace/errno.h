#ifndef HP_ERRNO_H
#define HP_ERRNO_H

/* 成功 */
#define HP_SUCCESS       0

/* 错误码 */
#define HP_EINVAL        (-1)   /* 无效参数 */
#define HP_ENOMEM        (-2)   /* 内存不足 */
#define HP_EBUSY         (-3)   /* 资源忙 */
#define HP_EPERM         (-4)   /* 操作不允许 */
#define HP_EEXIST        (-5)   /* 已存在 */
#define HP_ENOSYS        (-6)   /* 功能未实现 */
#define HP_ENOTSUP       (-7)   /* 不支持 */
#define HP_EALREADY      (-8)   /* 已经初始化 */

#endif /* HP_ERRNO_H */