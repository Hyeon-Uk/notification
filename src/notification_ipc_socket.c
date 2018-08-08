/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "notification_ipc.h"
#include "notification_debug.h"

#define RCV_SOCK 0
#define SND_SOCK 1

#define MAX_RETRY_CNT 10
#define WRITE_TIMEOUT 20 /* milliseconds*/

EXPORT_API int notification_ipc_socket_pair(int *fd)
{
	int ret_fd[2];
	int err;

	if (fd == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	errno = 0;
	err = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, ret_fd);
	if (err < 0) {
		ERR("socketpair [%d]", errno);
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	fd[RCV_SOCK] = ret_fd[RCV_SOCK];
	fd[SND_SOCK] = ret_fd[SND_SOCK];

	return NOTIFICATION_ERROR_NONE;
}

static int __get_socket_buffer_size(int fd, unsigned int *size, int optname)
{
	unsigned int ret_size = 0;
	socklen_t len = sizeof(ret_size);

	errno = 0;
	if (getsockopt(fd, SOL_SOCKET, optname, &ret_size, &len) < 0) {
		ERR("read socket size [%d]", errno);
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	*size = ret_size;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_ipc_socket_get_read_buf_size(int fd, unsigned int *size)
{
	return __get_socket_buffer_size(fd, size, SO_RCVBUF);
}

EXPORT_API int notification_ipc_socket_get_write_buf_size(int fd, unsigned int *size)
{
	return __get_socket_buffer_size(fd, size, SO_SNDBUF);
}


EXPORT_API int notification_ipc_socket_write(int fd, const char *buffer, unsigned int nbytes)
{
	int retry_cnt = 0;
	unsigned int left = nbytes;
	ssize_t nb;
	const struct timespec SLEEP_TIME = { 0, 20 * 1000 * 1000 };

	while (left && (retry_cnt < MAX_RETRY_CNT)) {
		errno = 0;
		nb = write(fd, buffer, left);
		if (nb == -1) {
			/* LCOV_EXCL_START */
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
				ERR("continue..");
				retry_cnt++;
				nanosleep(&SLEEP_TIME, 0);
				continue;
			}
			ERR("error fd [%d] errno [%d]", fd, errno);
			return NOTIFICATION_ERROR_IO_ERROR;
			/* LCOV_EXCL_STOP */
		}

		left -= nb;
		buffer += nb;
		retry_cnt = 0;
	}

	if (left != 0) {
		ERR("error fd [%d], retry_cnt [%d]", fd, retry_cnt);
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_ipc_socket_write_string(int fd, const char *buffer, unsigned int string_len)
{
	int ret;

	ret = notification_ipc_socket_write(fd, (char *)&string_len, sizeof(string_len));
	if (ret != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("write string_len fail");
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (string_len > 0) {
		ret = notification_ipc_socket_write(fd, buffer, string_len);
		if (ret != NOTIFICATION_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("write string fail");
			return ret;
			/* LCOV_EXCL_STOP */
		}
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_ipc_socket_read(int fd, char *buffer, unsigned int nbytes)
{
	unsigned int left = nbytes;
	ssize_t nb;
	int retry_cnt = 0;
	const struct timespec SLEEP_TIME = { 0, 20 * 1000 * 1000 };

	while (left && (retry_cnt < MAX_RETRY_CNT)) {
		errno = 0;
		nb = read(fd, buffer, left);
		if (nb == 0) {
			ERR("read socket - EOF, fd close [%d]", fd);
			return NOTIFICATION_ERROR_IO_ERROR;
		} else if (nb == -1) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
				ERR("errno [%d], sleep and retry", errno);
				retry_cnt++;
				nanosleep(&SLEEP_TIME, 0);
				continue;
			}
			ERR("errno [%d] fd [%d]", errno, fd);
			return NOTIFICATION_ERROR_IO_ERROR;
		}
		left -= nb;
		buffer += nb;
		retry_cnt = 0;
	}

	if (left != 0) {
		ERR("error fd [%d] retry_cnt [%d]", fd, retry_cnt);
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	return NOTIFICATION_ERROR_NONE;
}
