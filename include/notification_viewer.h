/*
 * Copyright (c) 2000 - 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __NOTIFICATION_VIEWER_H__
#define __NOTIFICATION_VIEWER_H__

int notification_get_default_viewer(const char *path, char **default_viewer);
int notification_launch_default_viewer(const char *default_viewer, int priv_id,
				notification_op_type_e status, uid_t uid);

#endif /* __NOTIFICATION_VIEWER_H__ */
