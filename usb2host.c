/*
    usb2host: a tiny program to put dual-role ports to host mode
    Copyright (C) 2023-present Guoxin "7Ji" Pu

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#define SYSFS_PARENT "/sys/kernel/debug/usb"
#define SYSFS_FILE  "mode"
#define pr_error(format, arg...) fprintf(stderr, format, ##arg)
#define pr_error_with_errno(format, arg...) \
    pr_error(format ", errno: %d (%s)\n", ##arg, errno, strerror(errno))

static inline int format_path(
    char *const restrict path, 
    char const *const restrict node
) {
    unsigned short const len = strlen(node);
    if (!len) {
        pr_error("Node name is empty\n");
        return -1;
    }
    memcpy(path, node, len);
    memcpy(path + len + 1, SYSFS_FILE, sizeof SYSFS_FILE - 1);
    path[len] = '/';
    path[len + sizeof SYSFS_FILE] = '\0';
    return 0;
}

static inline int write_host(int const fd) {
    ssize_t written = write(fd, "host", 4);
    switch (written) {
    case 4:
        return 0;
    case -1:
        pr_error_with_errno("Failed to write 'host' into fd");
        return -1;
    default:
        pr_error("Written bytes unexpected: %ld instead of 4\n", written);
        return -1;
    }
}

static inline int write_node(int const atfd, char const node[]) {
    char path[128];
    if (format_path(path, node)) return -1;
    int const fd = openat(atfd, path, O_WRONLY);
    if (fd < 0) {
        pr_error_with_errno("Failed to open file '%s'", path);
        return -1;
    }
    int r = write_host(fd);
    if (close(fd)) {
        pr_error_with_errno("Failed to close file '%s'", path);
        r = -1;
    }
    return r;
}

// For performance, this does not call write_node()
static inline int write_all_nodes(int const atfd) {
    int const dir_fd = dup(atfd);
    if (dir_fd < 0) {
        pr_error_with_errno("Failed to dup fd");
        return -1;
    }
    DIR *const restrict dir = fdopendir(dir_fd);
    if (!dir) {
        pr_error_with_errno("Failed to opendir from fd");
        return -1;
    }
    struct dirent *entry;
    errno = 0;
    int r = 0;
    while ((entry = readdir(dir))) {
        switch (entry->d_name[0]) {
        case '\0':
            pr_error("Empty dir entry name encountered\n");
            r = -1;
            break;
        case '.':
            switch (entry->d_name[1]) {
            case '\0': // '.' dir itself
                continue;
            case '.':  // '..' parent
                if (entry->d_name[2 == '\0']) continue;
            }
            break;
        }
        if (entry->d_type != DT_DIR) continue;
        char path[128];
        if (format_path(path, entry->d_name)) {
            r = -1;
            continue;
        }
        int errno_backup = errno;
        int const fd = openat(atfd, path, O_WRONLY);
        if (fd < 0) {
            if (errno != ENOENT) {
                pr_error_with_errno("Failed to open file '%s'", path);
                r = -1;
            }
            errno = errno_backup;
            continue;
        }
        if (write_host(fd)) {
            pr_error("Failed to write host mode to file '%s'\n", path);
            r = -1;
        }
        if (close(fd)) {
            pr_error_with_errno("Failed to close file '%s'", path);
            r = -1;
        }
        errno = errno_backup;
    }
    if (errno) {
        pr_error_with_errno("Failed to readdir");
        r = -1;
    }
    if (closedir(dir)) {
        pr_error_with_errno("Failed to closedir");
        r = -1;
    }
    return r;
}

int main(int const argc, char const *argv[]) {
    int fd = open(SYSFS_PARENT, O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        pr_error_with_errno("Failed to open sysfs dir '"SYSFS_PARENT"'");
        return -1;
    }
    int r;
    if (argc > 1) {
        r = 0;
        for (int i = 1; i < argc; ++i) {
            if (write_node(fd, argv[1])) {
                pr_error("Failed to put node '%s' to host mode\n", argv[1]);
                r = -1;
            }
        }
    } else {
        pr_error("WARNING: no nodes given, trying to put all usb nodes under '"
                SYSFS_PARENT"' into host mode\n");
        r = write_all_nodes(fd);
    }
    if (close(fd)) {
        pr_error_with_errno("Failed to close sysfs dir '"SYSFS_PARENT"'");
        r = -1;
    }
    return r;
}